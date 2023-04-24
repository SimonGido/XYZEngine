#include "stdafx.h"
#include "VulkanShader.h"

#include "XYZ/Renderer/Renderer.h"
#include "XYZ/Renderer/Material.h"
#include "XYZ/Renderer/Pipeline.h"
#include "XYZ/Renderer/ShaderIncluder.h"
#include "XYZ/Renderer/ShaderCIncluder.h"

#include "XYZ/Utils/StringUtils.h"
#include "XYZ/Utils/FileSystem.h"


#include "VulkanContext.h"
#include "VulkanRendererAPI.h"
#include "Vulkan.h"

#include <glm/gtc/type_ptr.hpp>

#include <shaderc/shaderc.hpp>
#include <spirv_cross/spirv_cross.hpp>
#include <spirv_cross/spirv_glsl.hpp>

#include <fstream>
#include <array>
#include <filesystem>


namespace XYZ {

	namespace Utils {
		
		// Custom shader keywords
		static constexpr const char* sc_InstancedKeyword = "XYZ_INSTANCED";

		
		static void PrintResources(
			const spirv_cross::Compiler& compiler, 
			const char* tag, 
			const spirv_cross::SmallVector<spirv_cross::Resource>& resources
		)
		{
			fprintf(stderr, "%s\n", tag);
			fprintf(stderr, "=============\n\n");
			bool print_ssbo = !strcmp(tag, "ssbos");

			for (auto& res : resources)
			{
				auto& type = compiler.get_type(res.type_id);

				if (print_ssbo && compiler.buffer_is_hlsl_counter_buffer(res.id))
					continue;

				// If we don't have a name, use the fallback for the type instead of the variable
				// for SSBOs and UBOs since those are the only meaningful names to use externally.
				// Push constant blocks are still accessed by name and not block name, even though they are technically Blocks.
				bool is_push_constant = compiler.get_storage_class(res.id) == spv::StorageClassPushConstant;
				bool is_block = compiler.get_decoration_bitset(type.self).get(spv::DecorationBlock) ||
					compiler.get_decoration_bitset(type.self).get(spv::DecorationBufferBlock);
				bool is_sized_block = is_block && (compiler.get_storage_class(res.id) == spv::StorageClassUniform ||
					compiler.get_storage_class(res.id) == spv::StorageClassUniformConstant);
				spirv_cross::ID fallback_id = !is_push_constant && is_block ? spirv_cross::ID(res.base_type_id) : spirv_cross::ID(res.id);

				uint32_t block_size = 0;
				uint32_t runtime_array_stride = 0;
				if (is_sized_block)
				{
					auto& base_type = compiler.get_type(res.base_type_id);
					block_size = uint32_t(compiler.get_declared_struct_size(base_type));
					runtime_array_stride = uint32_t(compiler.get_declared_struct_size_runtime_array(base_type, 1) -
						compiler.get_declared_struct_size_runtime_array(base_type, 0));
				}

				spirv_cross::Bitset mask;
				if (print_ssbo)
					mask = compiler.get_buffer_block_flags(res.id);
				else
					mask = compiler.get_decoration_bitset(res.id);


				if (mask.get(spv::DecorationLocation))
					fprintf(stderr, " (Location : %u)", compiler.get_decoration(res.id, spv::DecorationLocation));
				if (mask.get(spv::DecorationDescriptorSet))
					fprintf(stderr, " (Set : %u)", compiler.get_decoration(res.id, spv::DecorationDescriptorSet));
				if (mask.get(spv::DecorationBinding))
					fprintf(stderr, " (Binding : %u)", compiler.get_decoration(res.id, spv::DecorationBinding));
				if (static_cast<const spirv_cross::CompilerGLSL&>(compiler).variable_is_depth_or_compare(res.id))
					fprintf(stderr, " (comparison)");
				if (mask.get(spv::DecorationInputAttachmentIndex))
					fprintf(stderr, " (Attachment : %u)", compiler.get_decoration(res.id, spv::DecorationInputAttachmentIndex));
				if (mask.get(spv::DecorationNonReadable))
					fprintf(stderr, " writeonly");
				if (mask.get(spv::DecorationNonWritable))
					fprintf(stderr, " readonly");
				if (mask.get(spv::DecorationRestrict))
					fprintf(stderr, " restrict");
				if (mask.get(spv::DecorationCoherent))
					fprintf(stderr, " coherent");
				if (mask.get(spv::DecorationVolatile))
					fprintf(stderr, " volatile");
				if (is_sized_block)
				{
					fprintf(stderr, " (BlockSize : %u bytes)", block_size);
					if (runtime_array_stride)
						fprintf(stderr, " (Unsized array stride: %u bytes)", runtime_array_stride);
				}

				uint32_t counter_id = 0;
				if (print_ssbo && compiler.buffer_get_hlsl_counter_buffer(res.id, counter_id))
					fprintf(stderr, " (HLSL counter buffer ID: %u)", counter_id);
				fprintf(stderr, "\n");
			}
			fprintf(stderr, "=============\n\n");
		}


		static ShaderDataType SPIRTypeToShaderDataType(spirv_cross::SPIRType type)
		{
			switch (type.basetype)
			{
			case spirv_cross::SPIRType::Boolean:  return ShaderDataType::Bool;
			case spirv_cross::SPIRType::Int:
				if (type.vecsize == 1)            return ShaderDataType::Int;
				if (type.vecsize == 2)            return ShaderDataType::Int2;
				if (type.vecsize == 3)            return ShaderDataType::Int3;
				if (type.vecsize == 4)            return ShaderDataType::Int4;

			//case spirv_cross::SPIRType::UInt:     return ShaderDataType::UInt;
			case spirv_cross::SPIRType::Float:
				if (type.columns == 3)            return ShaderDataType::Mat3;
				if (type.columns == 4)            return ShaderDataType::Mat4;

				if (type.vecsize == 1)            return ShaderDataType::Float;
				if (type.vecsize == 2)            return ShaderDataType::Float2;
				if (type.vecsize == 3)            return ShaderDataType::Float3;
				if (type.vecsize == 4)            return ShaderDataType::Float4;
				break;
			}
			XYZ_ASSERT(false, "Unknown type!");
			return ShaderDataType::None;
		}


		static ShaderUniformDataType SPIRTypeToShaderUniformType(spirv_cross::SPIRType type)
		{
			switch (type.basetype)
			{
			case spirv_cross::SPIRType::Boolean:  return ShaderUniformDataType::Bool;
			case spirv_cross::SPIRType::Int:
				if (type.vecsize == 1)            return ShaderUniformDataType::Int;
				if (type.vecsize == 2)            return ShaderUniformDataType::IntVec2;
				if (type.vecsize == 3)            return ShaderUniformDataType::IntVec3;
				if (type.vecsize == 4)            return ShaderUniformDataType::IntVec4;

			case spirv_cross::SPIRType::UInt:     return ShaderUniformDataType::UInt;
			case spirv_cross::SPIRType::Float:
				if (type.columns == 3)            return ShaderUniformDataType::Mat3;
				if (type.columns == 4)            return ShaderUniformDataType::Mat4;

				if (type.vecsize == 1)            return ShaderUniformDataType::Float;
				if (type.vecsize == 2)            return ShaderUniformDataType::Vec2;
				if (type.vecsize == 3)            return ShaderUniformDataType::Vec3;
				if (type.vecsize == 4)            return ShaderUniformDataType::Vec4;
				break;
			}
			XYZ_ASSERT(false, "Unknown type!");
			return ShaderUniformDataType::None;
		}

		static uint32_t SPIRTypeSize(spirv_cross::SPIRType type)
		{
			switch (type.basetype)
			{
			case spirv_cross::SPIRType::Boolean:  return 4;
			case spirv_cross::SPIRType::Int:
				if (type.vecsize == 1)            return 4;
				if (type.vecsize == 2)            return 2 * 4;
				if (type.vecsize == 3)            return 3 * 4;
				if (type.vecsize == 4)            return 4 * 4;

			case spirv_cross::SPIRType::UInt:     return 4;
			case spirv_cross::SPIRType::Float:
				if (type.columns == 3)            return 3 * 3 * 4;
				if (type.columns == 4)            return 4 * 4 * 4;

				if (type.vecsize == 1)            return 4;
				if (type.vecsize == 2)            return 2 * 4;
				if (type.vecsize == 3)            return 3 * 4;
				if (type.vecsize == 4)            return 4 * 4;
				break;
			}
			XYZ_ASSERT(false, "Unknown type!");
			return 0;
		}

		static const char* VkShaderStageCachedFileExtension(VkShaderStageFlagBits stage)
		{
			switch (stage)
			{
			case VK_SHADER_STAGE_VERTEX_BIT:    return ".cached_vulkan.vert";
			case VK_SHADER_STAGE_FRAGMENT_BIT:  return ".cached_vulkan.frag";
			case VK_SHADER_STAGE_COMPUTE_BIT:   return ".cached_vulkan.comp";
			}
			XYZ_ASSERT(false, "");
			return "";
		}

		static shaderc_shader_kind VkShaderStageToShaderC(VkShaderStageFlagBits stage)
		{
			switch (stage)
			{
			case VK_SHADER_STAGE_VERTEX_BIT:    return shaderc_vertex_shader;
			case VK_SHADER_STAGE_FRAGMENT_BIT:  return shaderc_fragment_shader;
			case VK_SHADER_STAGE_COMPUTE_BIT:   return shaderc_compute_shader;
			}
			XYZ_ASSERT(false, "");
			return (shaderc_shader_kind)0;
		}

		static ShaderType VkShaderStageToShaderType(VkShaderStageFlagBits stage)
		{
			switch (stage)
			{
			case VK_SHADER_STAGE_VERTEX_BIT:    return ShaderType::Vertex;
			case VK_SHADER_STAGE_FRAGMENT_BIT:  return ShaderType::Fragment;
			case VK_SHADER_STAGE_COMPUTE_BIT:   return ShaderType::Compute;
			}
			XYZ_ASSERT(false, "");
			return ShaderType::None;
		}

		static VkShaderStageFlagBits ShaderStageFromString(const std::string& type)
		{
			if (type == "vertex")
				return VK_SHADER_STAGE_VERTEX_BIT;
			if (type == "fragment" || type == "pixel")
				return VK_SHADER_STAGE_FRAGMENT_BIT;
			if (type == "compute")
				return VK_SHADER_STAGE_COMPUTE_BIT;

			XYZ_ASSERT(false, "Unknown shader type!");
			return VK_SHADER_STAGE_FLAG_BITS_MAX_ENUM;
		}

		static std::vector<BufferLayout> CreateBufferLayouts(
			const spirv_cross::Compiler& compiler,
			const spirv_cross::SmallVector<spirv_cross::Resource>& resources,
			const std::unordered_map<std::string, ShaderParser::ShaderLayoutInfo>& layoutInfos
		)
		{
			std::vector<BufferLayout> result;
			std::vector<BufferElement> elements;
			std::vector<BufferElement> elementsInstanced;

			for (const auto& res : resources)
			{
				auto& type = compiler.get_type(res.type_id);

				const spirv_cross::Bitset mask = compiler.get_decoration_bitset(res.id);

				const uint32_t location = compiler.get_decoration(res.id, spv::DecorationLocation);


				const ShaderDataType shaderDataType = SPIRTypeToShaderDataType(type);
				const ShaderParser::ShaderLayoutInfo& layoutInfo = layoutInfos.at(res.name);
				
				if (layoutInfo.Keyword == sc_InstancedKeyword)
				{
					elementsInstanced.emplace_back(location, shaderDataType, res.name);
				}
				else
				{
					elements.emplace_back(location, shaderDataType, res.name);
				}
			}

			std::sort(elements.begin(), elements.end(), [](const BufferElement& a, const BufferElement& b) {
				return a.Location < b.Location;
			});
		
			std::sort(elementsInstanced.begin(), elementsInstanced.end(), [](const BufferElement& a, const BufferElement& b) {
				return a.Location < b.Location;
			});

			if (!elements.empty())
				result.emplace_back(elements);
			if (!elementsInstanced.empty())
				result.emplace_back(elementsInstanced, true);
		
			return result;
		}

		static const char* GetCacheDirectory()
		{
			// TODO: make sure the assets directory is valid
			return "Resources/Cache/Shader/Vulkan";
		}

		static void CreateCacheDirectoryIfNeeded()
		{
			const std::string cacheDirectory = GetCacheDirectory();
			if (!std::filesystem::exists(cacheDirectory))
				std::filesystem::create_directories(cacheDirectory);
		}	

		static std::string GetCachePath(const std::string& name, VkShaderStageFlagBits stage)
		{
			std::string result = GetCacheDirectory();
			result += "/" + name + VkShaderStageCachedFileExtension(stage);
			return result;
		}	
	}




	VulkanShader::VulkanShader(const std::string& path, size_t sourceHash, bool forceCompile)
		:
		m_Compiled(false),
		m_Name(Utils::GetFilenameWithoutExtension(path)),
		m_FilePath(path),
		m_SourceHash(sourceHash),
		m_VertexBufferSize(0)
	{
		m_Parser.AddKeyword("XYZ_INSTANCED");
		Reload(forceCompile);
	}
	VulkanShader::VulkanShader(const std::string& name, const std::string& path, size_t sourceHash, bool forceCompile)
		:
		m_Compiled(false),
		m_Name(name),
		m_FilePath(path),
		m_SourceHash(sourceHash),
		m_VertexBufferSize(0)
	{
		m_Parser.AddKeyword("XYZ_INSTANCED");
		Reload(forceCompile);
	}
	VulkanShader::VulkanShader(const std::string& name, const std::string& vertexPath, const std::string& fragmentPath, size_t sourceHash, bool forceCompile)
		:
		m_Compiled(false),
		m_Name(name),
		m_FilePath(Utils::GetDirectoryPath(vertexPath) + "/" + name + ".glsl"),
		m_SourceHash(sourceHash),
		m_VertexBufferSize(0)
	{
		std::ofstream outfile(m_FilePath);
		outfile << "#type vertex\n" << FileSystem::ReadFile(vertexPath);

		outfile << "\n\r#type fragment\r";
		outfile << FileSystem::ReadFile(fragmentPath);

		outfile.close();

		m_Parser.AddKeyword("XYZ_INSTANCED");
		Reload(forceCompile);
	}
	VulkanShader::~VulkanShader()
	{
		Renderer::RemoveShaderDependency(GetHash());
		destroy();
	}
	void VulkanShader::Reload(bool forceCompile)
	{
		destroy();
		
		m_PipelineShaderStageCreateInfos.clear();
		m_DescriptorSets.clear();
		m_PushConstantRanges.clear();
		m_Buffers.clear();
		m_Specializations.clear();

		Utils::CreateCacheDirectoryIfNeeded();

		m_Source = FileSystem::ReadFile(m_FilePath);
		PreprocessData preprocessData = preProcess(m_Source);
					

		const size_t newSourceHash = std::hash<std::string>{}(m_Source);
		if (newSourceHash != m_SourceHash) // Source code has changed
		{
			forceCompile = true;
			m_SourceHash = newSourceHash;
		}
		if (!forceCompile)
		{
			forceCompile = !binaryExists(preprocessData.Sources); // If binary does not exist recompile
		}
		if (forceCompile)
		{
			XYZ_CORE_INFO("Compiling shader {}", m_Name);
		}

		m_ShaderData = compileOrGetVulkanBinaries(preprocessData.Sources, forceCompile);
		reflectAllStages(m_ShaderData, preprocessData);
		createProgram(m_ShaderData);
		createDescriptorSetLayout();
		m_Compiled = true;
		Renderer::OnShaderReload(GetHash());		
	}


	size_t VulkanShader::GetHash() const
	{		
		return std::hash<std::string>{}(m_FilePath);
	}

	const std::vector<uint32_t>& VulkanShader::GetShaderData(ShaderType type) const
	{
		switch (type)
		{
		case XYZ::ShaderType::Vertex:
			return m_ShaderData.at(VkShaderStageFlagBits::VK_SHADER_STAGE_VERTEX_BIT);
		case XYZ::ShaderType::Fragment:
			return m_ShaderData.at(VkShaderStageFlagBits::VK_SHADER_STAGE_FRAGMENT_BIT);;
		case XYZ::ShaderType::Compute:
			return m_ShaderData.at(VkShaderStageFlagBits::VK_SHADER_STAGE_COMPUTE_BIT);
		}
		XYZ_ASSERT(false, "Shader does not have shader type");
		return m_ShaderData.at(VkShaderStageFlagBits::VK_SHADER_STAGE_ALL);
	}

	bool VulkanShader::IsCompute() const
	{
		return m_ShaderData.find(VkShaderStageFlagBits::VK_SHADER_STAGE_COMPUTE_BIT) != m_ShaderData.end();
	}

	bool VulkanShader::IsCompiled() const
	{
		return m_Compiled;
	}


	void VulkanShader::reflectAllStages(const VulkanShader::StageMap<std::vector<uint32_t>>& shaderData, const PreprocessData& preprocessData)
	{
		XYZ_TRACE("===========================");
		XYZ_TRACE(" Vulkan Shader Reflection");
		XYZ_TRACE(" {0}", m_FilePath);
		XYZ_TRACE("===========================");
		m_Resources.clear();

		for (auto [stage, data] : shaderData)
		{
			reflectStage(stage, data, preprocessData);
		}
	}

	void VulkanShader::reflectStage(VkShaderStageFlagBits stage, const std::vector<uint32_t>& shaderData, const PreprocessData& preprocessData)
	{
		const spirv_cross::Compiler compiler(shaderData);
		spirv_cross::ShaderResources resources = compiler.get_shader_resources();
		
		if (stage == VK_SHADER_STAGE_VERTEX_BIT)
		{
			m_Layouts = Utils::CreateBufferLayouts(compiler, resources.stage_inputs, preprocessData.LayoutInfo.at(stage));
		}

		reflectConstantBuffers(compiler, stage, resources.push_constant_buffers);
		reflectStorageBuffers(compiler, stage, resources.storage_buffers);
		reflectUniformBuffers(compiler, stage, resources.uniform_buffers);
		reflectSampledImages(compiler, stage, resources.sampled_images);
		reflectStorageImages(compiler, stage, resources.storage_images);
		reflectSpecializationConstants(compiler, stage, compiler.get_specialization_constants());

		if (stage == VK_SHADER_STAGE_VERTEX_BIT || stage == VK_SHADER_STAGE_COMPUTE_BIT)
			m_VertexBufferSize = getBuffersSize();
	}
	void VulkanShader::reflectConstantBuffers(const spirv_cross::Compiler& compiler, VkShaderStageFlagBits stage, spirv_cross::SmallVector<spirv_cross::Resource>& buffers)
	{
		XYZ_TRACE("Push Constant Buffers:");
		for (const auto& resource : buffers)
		{
			const auto& bufferName = resource.name;
			auto& bufferType = compiler.get_type(resource.base_type_id);
			auto bufferSize = (uint32_t)compiler.get_declared_struct_size(bufferType);
			uint32_t memberCount = uint32_t(bufferType.member_types.size());
			uint32_t bufferOffset = 0;
			if (m_PushConstantRanges.size())
				bufferOffset = m_PushConstantRanges.back().Offset + m_PushConstantRanges.back().Size;

			auto& pushConstantRange = m_PushConstantRanges.emplace_back();
			pushConstantRange.ShaderStage = stage;
			pushConstantRange.Size = bufferSize - bufferOffset;
			pushConstantRange.Offset = bufferOffset;


			ShaderBuffer& buffer = m_Buffers[bufferName];
			buffer.Name = bufferName;
			buffer.Size = bufferSize - bufferOffset;

			XYZ_TRACE("  Name: {0}", bufferName);
			XYZ_TRACE("  Member Count: {0}", memberCount);
			XYZ_TRACE("  Size: {0}", bufferSize);

			for (uint32_t i = 0; i < memberCount; i++)
			{
				const auto type = compiler.get_type(bufferType.member_types[i]);
				const auto& memberName = compiler.get_member_name(bufferType.self, i);
				const auto size = (uint32_t)compiler.get_declared_struct_member_size(bufferType, i);
				const auto offset = compiler.type_struct_member_offset(bufferType, i);

				std::string uniformName = fmt::format("{}.{}", bufferName, memberName);
				buffer.Uniforms[uniformName] = ShaderUniform(uniformName, Utils::SPIRTypeToShaderUniformType(type), size, offset);
			}
		}
	}
	void VulkanShader::reflectStorageBuffers(const spirv_cross::Compiler& compiler, VkShaderStageFlagBits stage, spirv_cross::SmallVector<spirv_cross::Resource>& buffers)
	{
		XYZ_TRACE("Storage Buffers:");
		for (const auto& resource : buffers)
		{
			const auto& name = resource.name;
			auto& bufferType = compiler.get_type(resource.base_type_id);
			uint32_t memberCount = (uint32_t)bufferType.member_types.size();
			uint32_t binding = compiler.get_decoration(resource.id, spv::DecorationBinding);
			uint32_t descriptorSet = compiler.get_decoration(resource.id, spv::DecorationDescriptorSet);
			uint32_t size = (uint32_t)compiler.get_declared_struct_size(bufferType);


			if (descriptorSet >= m_DescriptorSets.size())
				m_DescriptorSets.resize(static_cast<size_t>(descriptorSet) + 1);
			ShaderDescriptorSet& shaderDescriptorSet = m_DescriptorSets[descriptorSet].ShaderDescriptorSet;
			
			
			StorageBuffer storageBuffer;
			storageBuffer.BindingPoint = binding;
			storageBuffer.Size = size;
			storageBuffer.Name = name;
			storageBuffer.ShaderStage = VK_SHADER_STAGE_ALL;
		
			shaderDescriptorSet.StorageBuffers[binding] = storageBuffer;

			XYZ_TRACE("  {0} ({1}, {2})", name, descriptorSet, binding);
			XYZ_TRACE("  Member Count: {0}", memberCount);
			XYZ_TRACE("  Size: {0}", size);
			XYZ_TRACE("-------------------");
		}
	}
	void VulkanShader::reflectUniformBuffers(const spirv_cross::Compiler& compiler, VkShaderStageFlagBits stage, spirv_cross::SmallVector<spirv_cross::Resource>& buffers)
	{
		XYZ_TRACE("Uniform Buffers:");
		for (const auto& resource : buffers)
		{
			const auto& name = resource.name;
			auto& bufferType = compiler.get_type(resource.base_type_id);
			int memberCount = (uint32_t)bufferType.member_types.size();
			uint32_t binding = compiler.get_decoration(resource.id, spv::DecorationBinding);
			uint32_t descriptorSet = compiler.get_decoration(resource.id, spv::DecorationDescriptorSet);
			uint32_t size = (uint32_t)compiler.get_declared_struct_size(bufferType);

			if (descriptorSet >= m_DescriptorSets.size())
				m_DescriptorSets.resize(static_cast<size_t>(descriptorSet) + 1);
			ShaderDescriptorSet& shaderDescriptorSet = m_DescriptorSets[descriptorSet].ShaderDescriptorSet;

		
			UniformBuffer uniformBuffer;
			uniformBuffer.BindingPoint = binding;
			uniformBuffer.Size = size;
			uniformBuffer.Name = name;
			uniformBuffer.ShaderStage = VK_SHADER_STAGE_ALL;
			
			shaderDescriptorSet.UniformBuffers[binding] = uniformBuffer;

			XYZ_TRACE("  {0} ({1}, {2})", name, descriptorSet, binding);
			XYZ_TRACE("  Member Count: {0}", memberCount);
			XYZ_TRACE("  Size: {0}", size);
			XYZ_TRACE("-------------------");
		}
	}
	void VulkanShader::reflectSampledImages(const spirv_cross::Compiler& compiler, VkShaderStageFlagBits stage, spirv_cross::SmallVector<spirv_cross::Resource>& sampledImages)
	{
		XYZ_TRACE("Sampled Images:");
		for (const auto& resource : sampledImages)
		{
			const auto& name = resource.name;
			auto& baseType = compiler.get_type(resource.base_type_id);
			auto& type = compiler.get_type(resource.type_id);
			uint32_t binding = compiler.get_decoration(resource.id, spv::DecorationBinding);
			uint32_t descriptorSet = compiler.get_decoration(resource.id, spv::DecorationDescriptorSet);
			uint32_t dimension = baseType.image.dim;
			uint32_t arraySize = type.array[0];
			if (arraySize == 0)
				arraySize = 1;
			
			if (descriptorSet >= m_DescriptorSets.size())
				m_DescriptorSets.resize(static_cast<size_t>(descriptorSet) + 1);
			ShaderDescriptorSet& shaderDescriptorSet = m_DescriptorSets[descriptorSet].ShaderDescriptorSet;
			
			auto& imageSampler = shaderDescriptorSet.ImageSamplers[binding];
			imageSampler.BindingPoint = binding;
			imageSampler.DescriptorSet = descriptorSet;
			imageSampler.Name = name;
			imageSampler.ShaderStage = stage;
			imageSampler.ArraySize = arraySize;

			m_Resources[name] = ShaderResourceDeclaration(name, binding, arraySize, ShaderResourceType::Sampler2D);

			XYZ_TRACE("  {0} ({1}, {2})", name, descriptorSet, binding);
		}
	}
	void VulkanShader::reflectStorageImages(const spirv_cross::Compiler& compiler, VkShaderStageFlagBits stage, spirv_cross::SmallVector<spirv_cross::Resource>& storageImages)
	{
		XYZ_TRACE("Storage Images:");
		for (const auto& resource : storageImages)
		{
			const auto& name = resource.name;
			auto& type = compiler.get_type(resource.base_type_id);
			uint32_t binding = compiler.get_decoration(resource.id, spv::DecorationBinding);
			uint32_t descriptorSet = compiler.get_decoration(resource.id, spv::DecorationDescriptorSet);
			uint32_t dimension = type.image.dim;

			if (descriptorSet >= m_DescriptorSets.size())
				m_DescriptorSets.resize(static_cast<size_t>(descriptorSet) + 1);
			ShaderDescriptorSet& shaderDescriptorSet = m_DescriptorSets[descriptorSet].ShaderDescriptorSet;
			
			auto& imageSampler = shaderDescriptorSet.StorageImages[binding];
			imageSampler.BindingPoint = binding;
			imageSampler.DescriptorSet = descriptorSet;
			imageSampler.Name = name;
			imageSampler.ShaderStage = stage;

			m_Resources[name] = ShaderResourceDeclaration(name, binding, 1);

			XYZ_TRACE("  {0} ({1}, {2})", name, descriptorSet, binding);
		}
	}
	void VulkanShader::reflectSpecializationConstants(const spirv_cross::Compiler& compiler, VkShaderStageFlagBits stage, spirv_cross::SmallVector<spirv_cross::SpecializationConstant>& specializationConstants)
	{
		uint32_t offset = 0;		
		for (auto& specialization : specializationConstants)
		{
			const spirv_cross::SPIRConstant& constant = compiler.get_constant(specialization.id);
			auto& type = compiler.get_type(constant.constant_type);
			std::string name = compiler.get_name(specialization.id);
			
			
			auto& spec = m_Specializations[name];
			spec.Size = Utils::SPIRTypeSize(type);
			spec.Offset = offset;
			spec.Type = Utils::VkShaderStageToShaderType(stage);
			spec.ConstantID = specialization.constant_id;
			offset += spec.Size;
		}
	}
	const VkPipelineShaderStageCreateInfo* VulkanShader::GetPipelineVertexShaderStageCreateInfo() const
	{
		for (const auto& stage : m_PipelineShaderStageCreateInfos)
		{
			if (stage.stage == VK_SHADER_STAGE_VERTEX_BIT)
			{
				return &stage;
			}
		}
		return nullptr;
	}
	const VkWriteDescriptorSet* VulkanShader::GetDescriptorSet(const std::string& name, uint32_t set) const
	{
		XYZ_ASSERT(set < m_DescriptorSets.size(), "");
		XYZ_ASSERT(m_DescriptorSets[set].ShaderDescriptorSet, "");
		auto& shaderDescriptorSet = m_DescriptorSets.at(set).ShaderDescriptorSet;

		auto it = shaderDescriptorSet.WriteDescriptorSets.find(name);
		if (it == shaderDescriptorSet.WriteDescriptorSets.end())
		{
			XYZ_CORE_WARN("Shader {0} does not contain requested descriptor set {1}", m_Name, name);
			return nullptr;
		}
		return &it->second;
	}
	std::pair<const VkWriteDescriptorSet*, uint32_t> VulkanShader::GetDescriptorSet(const std::string& name) const
	{
		for (uint32_t set = 0; set < m_DescriptorSets.size(); ++set)
		{
			auto& shaderDescriptorSet = m_DescriptorSets.at(set).ShaderDescriptorSet;
			
			auto it = shaderDescriptorSet.WriteDescriptorSets.find(name);
			if (it == shaderDescriptorSet.WriteDescriptorSets.end())
				continue;
			
			return { &it->second, set };
		}
		XYZ_CORE_WARN("Shader {0} does not contain requested descriptor set {1}", m_Name, name);
		return { nullptr, 0 };
	}
	const VkWriteDescriptorSet* VulkanShader::TryGetDescriptorSet(const std::string& name, uint32_t set) const
	{
		if (set >= m_DescriptorSets.size())
			return nullptr;

		if (!m_DescriptorSets[set].ShaderDescriptorSet)
			return nullptr;

		auto& shaderDescriptorSet = m_DescriptorSets.at(set).ShaderDescriptorSet;
		auto it = shaderDescriptorSet.WriteDescriptorSets.find(name);
		
		if (it == shaderDescriptorSet.WriteDescriptorSets.end())
		{
			return nullptr;
		}
		return &it->second;
	}
	std::pair<const VkWriteDescriptorSet*, uint32_t> VulkanShader::TryGetDescriptorSet(const std::string& name) const
	{
		for (uint32_t set = 0; set < m_DescriptorSets.size(); ++set)
		{
			auto& shaderDescriptorSet = m_DescriptorSets.at(set).ShaderDescriptorSet;

			auto it = shaderDescriptorSet.WriteDescriptorSets.find(name);
			if (it == shaderDescriptorSet.WriteDescriptorSets.end())
				continue;

			return { &it->second, set };
		}

		return { nullptr, 0 };
	}
	std::vector<VkDescriptorSetLayout> VulkanShader::GetAllDescriptorSetLayouts() const
	{
		std::vector<VkDescriptorSetLayout> result;
		result.reserve(m_DescriptorSets.size());
		for (auto& descrSet : m_DescriptorSets)
			result.emplace_back(descrSet.DescriptorSetLayout);

		return result;
	}
	VulkanShader::StageMap<std::vector<uint32_t>> VulkanShader::compileOrGetVulkanBinaries(const VulkanShader::StageMap<std::string>& shaderSources, bool forceCompile)
	{
		StageMap<std::vector<uint32_t>> outputBinary;
		std::filesystem::path cacheDirectory = Utils::GetCacheDirectory();
		for (auto [stage, source] : shaderSources)
		{
			std::filesystem::path shaderFilePath = m_FilePath;
			std::filesystem::path cachedPath = cacheDirectory / (shaderFilePath.filename().string() + Utils::VkShaderStageCachedFileExtension(stage));

			if (!forceCompile)
			{
				std::ifstream in(cachedPath, std::ios::in | std::ios::binary);
				if (in.is_open())
				{
					in.seekg(0, std::ios::end);
					auto size = in.tellg();
					in.seekg(0, std::ios::beg);

					auto& data = outputBinary[stage];
					data.resize(size / sizeof(uint32_t));
					in.read((char*)data.data(), size);
					continue;
				}
			}
			else
			{
				// Do we need to init a compiler for each stage?
				shaderc::Compiler compiler;
				shaderc::CompileOptions options;

				options.SetIncluder(ShaderCIncluder::Create(Renderer::GetDefaultResources().Includer));

				options.SetTargetEnvironment(shaderc_target_env_vulkan, shaderc_env_version_vulkan_1_2);
				options.SetWarningsAsErrors();
				options.SetGenerateDebugInfo();

#ifdef XYZ_RELEASE
					options.SetOptimizationLevel(shaderc_optimization_level_performance);
#endif
				// Compile shader
				{
					auto& shaderSource = shaderSources.at(stage);
					shaderc::SpvCompilationResult module = compiler.CompileGlslToSpv(shaderSource, Utils::VkShaderStageToShaderC(stage), m_FilePath.c_str(), options);

					if (module.GetCompilationStatus() != shaderc_compilation_status_success)
					{
						XYZ_CORE_ERROR(module.GetErrorMessage());
					}
					outputBinary[stage] = std::vector<uint32_t>(module.cbegin(), module.cend());
				}
				// Cache compiled shader
				std::ofstream out(cachedPath, std::ios::out | std::ios::binary);
				if (out.is_open())
				{
					auto& data = outputBinary[stage];
					out.write((char*)data.data(), data.size() * sizeof(uint32_t));
					out.flush();
					out.close();
				}
			}
		}
		return outputBinary;
	}
	void VulkanShader::createProgram(const StageMap<std::vector<uint32_t>>& shaderData)
	{
		const VkDevice device = VulkanContext::GetCurrentDevice()->GetVulkanDevice();
		m_PipelineShaderStageCreateInfos.clear();

		for (auto [stage, data] : shaderData)
		{
			XYZ_ASSERT(data.size(), "");
			VkShaderModuleCreateInfo moduleCreateInfo{};
			
			moduleCreateInfo.sType = VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO;
			moduleCreateInfo.codeSize = data.size() * sizeof(uint32_t);
			moduleCreateInfo.pCode = data.data();
			
			VkShaderModule shaderModule;
			VK_CHECK_RESULT(vkCreateShaderModule(device, &moduleCreateInfo, NULL, &shaderModule));

			VkPipelineShaderStageCreateInfo& shaderStage = m_PipelineShaderStageCreateInfos.emplace_back();
			shaderStage.sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
			shaderStage.stage = stage;
			shaderStage.module = shaderModule;
			shaderStage.pName = "main";
		}
	}

	VulkanShader::PreprocessData VulkanShader::preProcess(const std::string& source) const
	{
		PreprocessData result;

		auto stageMap = m_Parser.ParseStages(source);

		std::vector<ShaderParser::ShaderLayoutInfo> vertexLayoutInfo;
		for (auto& [type, source] : stageMap)
		{
			auto stage = Utils::ShaderStageFromString(type);
			result.LayoutInfo[stage] = m_Parser.ParseLayoutInfo(source);
			m_Parser.RemoveKeywordsFromSourceCode(source);
			result.Sources[stage] = source;
		}

		return result;
	}


	void VulkanShader::createDescriptorSetLayout()
	{
		VkDevice device = VulkanContext::GetCurrentDevice()->GetVulkanDevice();

		for (uint32_t set = 0; set < m_DescriptorSets.size(); set++)
		{
			auto& shaderDescriptorSet = m_DescriptorSets[set].ShaderDescriptorSet;
			std::vector<VkDescriptorSetLayoutBinding> layoutBindings;
			for (auto& [binding, uniformBuffer] : shaderDescriptorSet.UniformBuffers)
			{
				VkDescriptorSetLayoutBinding& layoutBinding = layoutBindings.emplace_back();
				layoutBinding.descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
				layoutBinding.descriptorCount = 1;
				layoutBinding.stageFlags = uniformBuffer.ShaderStage;
				layoutBinding.pImmutableSamplers = nullptr;
				layoutBinding.binding = binding;

				VkWriteDescriptorSet& set = shaderDescriptorSet.WriteDescriptorSets[uniformBuffer.Name];
				set = {};
				set.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
				set.pNext = nullptr;
				set.descriptorType = layoutBinding.descriptorType;
				set.descriptorCount = 1;
				set.dstBinding = layoutBinding.binding;
			}

			for (auto& [binding, strorageBuffer] : shaderDescriptorSet.StorageBuffers)
			{
				VkDescriptorSetLayoutBinding& layoutBinding = layoutBindings.emplace_back();
				layoutBinding.descriptorType = VK_DESCRIPTOR_TYPE_STORAGE_BUFFER;
				layoutBinding.descriptorCount = 1;
				layoutBinding.stageFlags = strorageBuffer.ShaderStage;
				layoutBinding.pImmutableSamplers = nullptr;
				layoutBinding.binding = binding;

				VkWriteDescriptorSet& set = shaderDescriptorSet.WriteDescriptorSets[strorageBuffer.Name];
				set = {};
				set.pNext = nullptr;
				set.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
				set.descriptorType = layoutBinding.descriptorType;
				set.descriptorCount = 1;
				set.dstBinding = layoutBinding.binding;
			}

			for (auto& [binding, imageSampler] : shaderDescriptorSet.ImageSamplers)
			{
				auto& layoutBinding = layoutBindings.emplace_back();
				layoutBinding.descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
				layoutBinding.descriptorCount = imageSampler.ArraySize;
				layoutBinding.stageFlags = imageSampler.ShaderStage;
				layoutBinding.pImmutableSamplers = nullptr;
				layoutBinding.binding = binding;

				XYZ_ASSERT(shaderDescriptorSet.UniformBuffers.find(binding) == shaderDescriptorSet.UniformBuffers.end(), "Binding is already present!");
				XYZ_ASSERT(shaderDescriptorSet.StorageBuffers.find(binding) == shaderDescriptorSet.StorageBuffers.end(), "Binding is already present!");

				VkWriteDescriptorSet& set = shaderDescriptorSet.WriteDescriptorSets[imageSampler.Name];
				set = {};
				set.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
				set.descriptorType = layoutBinding.descriptorType;
				set.descriptorCount = imageSampler.ArraySize;
				set.dstBinding = layoutBinding.binding;
			}

			for (auto& [bindingAndSet, imageSampler] : shaderDescriptorSet.StorageImages)
			{
				auto& layoutBinding = layoutBindings.emplace_back();
				layoutBinding.descriptorType = VK_DESCRIPTOR_TYPE_STORAGE_IMAGE;
				layoutBinding.descriptorCount = 1;
				layoutBinding.stageFlags = imageSampler.ShaderStage;
				layoutBinding.pImmutableSamplers = nullptr;

				uint32_t binding = bindingAndSet & 0xffffffff;
				layoutBinding.binding = binding;

				XYZ_ASSERT(shaderDescriptorSet.UniformBuffers.find(binding) == shaderDescriptorSet.UniformBuffers.end(), "Binding is already present!");
				XYZ_ASSERT(shaderDescriptorSet.StorageBuffers.find(binding) == shaderDescriptorSet.StorageBuffers.end(), "Binding is already present!");
				XYZ_ASSERT(shaderDescriptorSet.ImageSamplers.find(binding) == shaderDescriptorSet.ImageSamplers.end(), "Binding is already present!");

				VkWriteDescriptorSet& set = shaderDescriptorSet.WriteDescriptorSets[imageSampler.Name];
				set = {};
				set.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
				set.descriptorType = layoutBinding.descriptorType;
				set.descriptorCount = 1;
				set.dstBinding = layoutBinding.binding;
			}

			VkDescriptorSetLayoutCreateInfo descriptorLayout = {};
			descriptorLayout.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO;
			descriptorLayout.pNext = nullptr;
			descriptorLayout.bindingCount = (uint32_t)(layoutBindings.size());
			descriptorLayout.pBindings = layoutBindings.data();
			
			XYZ_CORE_INFO("Creating descriptor set {0} with {1} ubo's, {2} ssbo's, {3} samplers and {4} storage images", set,
				shaderDescriptorSet.UniformBuffers.size(),
				shaderDescriptorSet.StorageBuffers.size(),
				shaderDescriptorSet.ImageSamplers.size(),
				shaderDescriptorSet.StorageImages.size()
			);
			if (set >= m_DescriptorSets.size())
				m_DescriptorSets.resize(static_cast<size_t>(set) + 1);
			
			VK_CHECK_RESULT(vkCreateDescriptorSetLayout(device, &descriptorLayout, nullptr, &m_DescriptorSets[set].DescriptorSetLayout));
		}		
	}


	void VulkanShader::destroy()
	{
		auto stageInfos = m_PipelineShaderStageCreateInfos;
		std::vector<VkDescriptorSetLayout> descriptorSetLayouts;
		descriptorSetLayouts.reserve(m_DescriptorSets.size());
		for (const auto& descrSet : m_DescriptorSets)
			descriptorSetLayouts.push_back(descrSet.DescriptorSetLayout);

		
			
		Renderer::SubmitResource([stageInfos, descriptorSetLayouts]() {
			const VkDevice device = VulkanContext::GetCurrentDevice()->GetVulkanDevice();
			for (const auto& info : stageInfos)
				vkDestroyShaderModule(device, info.module, nullptr);

			for (const auto& descr : descriptorSetLayouts)
				vkDestroyDescriptorSetLayout(device, descr, nullptr);
		});
		m_Compiled = false;
	}
	bool VulkanShader::binaryExists(const StageMap<std::string>& sources) const
	{
		std::filesystem::path cacheDirectory = Utils::GetCacheDirectory();
		std::filesystem::path shaderFilePath = m_FilePath;
		
		for (auto& [stage, source] : sources)
		{
			std::filesystem::path cachedPath = cacheDirectory / (shaderFilePath.filename().string() + Utils::VkShaderStageCachedFileExtension(stage));
			if (!std::filesystem::exists(cachedPath))
				return false;
		}
		return true;
	}
	
	size_t VulkanShader::getBuffersSize() const
	{
		size_t size = 0;
		for (auto& [name, buffer] : m_Buffers)
			size += buffer.Size;
		return size;
	}
}


