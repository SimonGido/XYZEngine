#include "stdafx.h"
#include "VulkanShader.h"

#include "XYZ/Renderer/Renderer.h"
#include "XYZ/Utils/StringUtils.h"
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


		static const char* GetCacheDirectory()
		{
			// TODO: make sure the assets directory is valid
			return "Assets/Cache/Shader/Vulkan";
		}

		static void CreateCacheDirectoryIfNeeded()
		{
			std::string cacheDirectory = GetCacheDirectory();
			if (!std::filesystem::exists(cacheDirectory))
				std::filesystem::create_directories(cacheDirectory);
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

		static VkShaderStageFlagBits ShaderComponentFromString(const std::string& type)
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
	}
	VulkanShader::VulkanShader(const std::string& path)
		:
		m_Name(Utils::GetFilenameWithoutExtension(path)),
		m_AssetPath(path)
	{
	}
	VulkanShader::VulkanShader(const std::string& name, const std::string& path)
		:
		m_Name(name),
		m_AssetPath(path)
	{
	}
	void VulkanShader::Reload(bool forceCompile)
	{
		std::string source = readFile(m_AssetPath);
		preProcess(source);
		std::unordered_map<VkShaderStageFlagBits, std::vector<uint32_t>> shaderData;
		compileOrGetVulkanBinaries(shaderData, forceCompile);
		for (auto&& [stage, data] : shaderData)
			reflect(stage, data);

		Ref<VulkanShader> instance = this;
		Renderer::SubmitAndWait([instance]() mutable {
			instance->createProgram();
		});

		for (size_t i = 0; i < m_ShaderReloadCallbacks.size(); ++i)
			m_ShaderReloadCallbacks[i]();
	}
	void VulkanShader::reflect(VkShaderStageFlagBits stage, const std::vector<uint32_t>& shaderData)
	{
	}
	void VulkanShader::compileOrGetVulkanBinaries(std::unordered_map<VkShaderStageFlagBits, std::vector<uint32_t>>& outputBinary, bool forceCompile)
	{
		std::filesystem::path cacheDirectory = Utils::GetCacheDirectory();
		for (auto [stage, source] : m_ShaderSources)
		{
			std::filesystem::path shaderFilePath = m_AssetPath;
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
				options.SetTargetEnvironment(shaderc_target_env_vulkan, shaderc_env_version_vulkan_1_2);
				options.SetWarningsAsErrors();
				options.SetGenerateDebugInfo();

				const bool optimize = false;
				if (optimize)
					options.SetOptimizationLevel(shaderc_optimization_level_performance);

				// Compile shader
				{
					auto& shaderSource = m_ShaderSources.at(stage);
					shaderc::SpvCompilationResult module = compiler.CompileGlslToSpv(shaderSource, Utils::VkShaderStageToShaderC(stage), m_AssetPath.c_str(), options);

					if (module.GetCompilationStatus() != shaderc_compilation_status_success)
					{
						XYZ_ERROR(module.GetErrorMessage());
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
	}
	void VulkanShader::createProgram(const std::unordered_map<VkShaderStageFlagBits, std::vector<uint32_t>>& shaderData)
	{
		VkDevice device = VulkanContext::GetCurrentDevice()->GetVulkanDevice();
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

	void VulkanShader::preProcess(const std::string& source)
	{
		const char* TypeToken = "#type";
		size_t TypeTokenLength = strlen(TypeToken);
		size_t pos = source.find(TypeToken, 0);
		while (pos != std::string::npos)
		{
			size_t eol = source.find_first_of("\r\n", pos);
			XYZ_ASSERT(eol != std::string::npos, "Syntax error");
			size_t begin = pos + TypeTokenLength + 1;
			std::string Type = source.substr(begin, eol - begin);
			VkShaderStageFlagBits stageType = Utils::ShaderComponentFromString(Type);
			XYZ_ASSERT(stageType, "Invalid shader Component specified");

			size_t nextLinePos = source.find_first_not_of("\r\n", eol);
			pos = source.find(TypeToken, nextLinePos);
			m_ShaderSources[stageType] = source.substr(nextLinePos, pos - (nextLinePos == std::string::npos ? source.size() - 1 : nextLinePos));
		}
	}
	std::string VulkanShader::readFile(const std::string& filepath) const
	{
		std::string result;
		std::ifstream in(filepath, std::ios::in | std::ios::binary); // ifstream closes itself due to RAII
		if (in)
		{
			in.seekg(0, std::ios::end);
			size_t size = in.tellg();
			if (size != -1)
			{
				result.resize(size);
				in.seekg(0, std::ios::beg);
				in.read(&result[0], size);
			}
			else
			{
				XYZ_ERROR("Could not read from file '{0}'", filepath);
			}
		}
		else
		{
			XYZ_ERROR("Could not open file '{0}'", filepath);
		}

		return result;
	}
}