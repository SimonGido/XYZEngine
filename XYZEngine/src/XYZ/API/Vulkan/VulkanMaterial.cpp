#include "stdafx.h"
#include "VulkanMaterial.h"

#include "XYZ/Utils/StringUtils.h"

namespace XYZ {

	namespace Utils {
		static uint32_t s_NextID = 0;
		static std::queue<uint32_t> s_FreeIDs;
		
		static uint32_t GenerateID()
		{
			if (!s_FreeIDs.empty())
			{
				uint32_t id = s_FreeIDs.back();
				s_FreeIDs.pop();
				return id;
			}
			return s_NextID++;
		}
	}

	VulkanMaterial::VulkanMaterial(const Ref<Shader>& shader)
		:
		m_Shader(shader),
		m_WriteDescriptors(Renderer::GetConfiguration().FramesInFlight),
		m_DescriptorsDirty(true),
		m_ID(Utils::GenerateID())
	{
		allocateStorage(m_Shader->GetBuffers(), m_UniformsBuffer);
		Ref<VulkanMaterial> instance = this;
		Renderer::Submit([instance]() mutable
		{
			instance->Invalidate();
		});
		Renderer::RegisterShaderDependency(shader, this);
	}
	VulkanMaterial::~VulkanMaterial()
	{
		m_UniformsBuffer.Destroy();
		Utils::s_FreeIDs.push(m_ID);
	}
	void VulkanMaterial::Invalidate()
	{
		allocateStorage(m_Shader->GetBuffers(), m_UniformsBuffer);
		m_DescriptorsDirty = true;
		Ref<VulkanShader> vulkanShader = m_Shader;
		const auto& shaderDescriptorSets = vulkanShader->GetDescriptorSets();
		m_DescriptorSets.resize(Renderer::GetConfiguration().FramesInFlight);
		
		m_ImageDescriptors.clear();
		m_ImageArraysDescriptors.clear();

		m_ImageDescriptors.resize(shaderDescriptorSets.size());
		m_ImageArraysDescriptors.resize(shaderDescriptorSets.size());


		for (auto& descriptorSet : m_DescriptorSets) // Per frame
		{
			descriptorSet.resize(shaderDescriptorSets.size());
			for (uint32_t set = 0; set < descriptorSet.size(); ++set)
				descriptorSet[set] = VulkanRendererAPI::RT_AllocateDescriptorSet(shaderDescriptorSets[set].DescriptorSetLayout);
		}
		m_DescriptorsVersion = VulkanRendererAPI::GetDescriptorAllocatorVersion();
	}
	void VulkanMaterial::SetFlag(RenderFlags renderFlag, bool val)
	{
		if (val)
		{
			m_Flags.Set(renderFlag);
		}
		else
		{
			m_Flags.Unset(renderFlag);
		}
	}
	void VulkanMaterial::Set(const std::string& name, float value)
	{
		set(name, value);
	}
	void VulkanMaterial::Set(const std::string& name, int value)
	{
		set(name, value);
	}
	void VulkanMaterial::Set(const std::string& name, const glm::vec2& value)
	{
		set(name, value);
	}
	void VulkanMaterial::Set(const std::string& name, const glm::vec3& value)
	{
		set(name, value);
	}
	void VulkanMaterial::Set(const std::string& name, const glm::vec4& value)
	{
		set(name, value);
	}
	void VulkanMaterial::Set(const std::string& name, const glm::mat4& value)
	{
		set(name, value);
	}
	void VulkanMaterial::Set(const std::string& name, const glm::ivec2& value)
	{
		set(name, value);
	}
	void VulkanMaterial::Set(const std::string& name, const glm::ivec3& value)
	{
		set(name, value);
	}
	void VulkanMaterial::Set(const std::string& name, const glm::ivec4& value)
	{
		set(name, value);
	}

	void VulkanMaterial::SetImageArray(const std::string& name, const Ref<Image2D>& image, uint32_t arrayIndex)
	{
		setDescriptor(name, arrayIndex, image);
	}

	void VulkanMaterial::SetImage(const std::string& name, const Ref<Image2D>& image, int32_t mip)
	{
		setDescriptor(name, image, mip);
	}

	float& VulkanMaterial::GetFloat(const std::string& name)
	{
		return get<float>(name);
	}
	int32_t& VulkanMaterial::GetInt(const std::string& name)
	{
		return get<int32_t>(name);
	}
	uint32_t& VulkanMaterial::GetUInt(const std::string& name)
	{
		return get<uint32_t>(name);
	}
	bool& VulkanMaterial::GetBool(const std::string& name)
	{
		return get<bool>(name);
	}
	glm::ivec2& VulkanMaterial::GetIVector2(const std::string& name)
	{
		return get<glm::ivec2>(name);
	}
	glm::ivec3& VulkanMaterial::GetIVector3(const std::string& name)
	{
		return get<glm::ivec3>(name);
	}
	glm::ivec4& VulkanMaterial::GetIVector4(const std::string& name)
	{
		return get<glm::ivec4>(name);
	}
	glm::vec2& VulkanMaterial::GetVector2(const std::string& name)
	{
		return get<glm::vec2>(name);
	}
	glm::vec3& VulkanMaterial::GetVector3(const std::string& name)
	{
		return get<glm::vec3>(name);
	}
	glm::vec4& VulkanMaterial::GetVector4(const std::string& name)
	{
		return get<glm::vec4>(name);
	}
	glm::mat3& VulkanMaterial::GetMatrix3(const std::string& name)
	{
		return get<glm::mat3>(name);
	}
	glm::mat4& VulkanMaterial::GetMatrix4(const std::string& name)
	{
		return get<glm::mat4>(name);
	}
	Ref<Texture2D> VulkanMaterial::GetTexture2D(const std::string& name)
	{
		return getResource<Texture2D>(name);
	}

	void VulkanMaterial::RT_UpdateForRendering(const vector3D<VkWriteDescriptorSet>& uniformBufferDescriptors, const vector3D<VkWriteDescriptorSet>& storageBufferDescriptors)
	{
		tryAllocateDescriptorSets();

		vector3D<VkDescriptorImageInfo> arrayImageInfos;
		if (m_DescriptorsDirty)
		{
			Ref<VulkanShader> vulkanShader = m_Shader;
			const uint32_t numSets = vulkanShader->GetDescriptorSets().size();
			const uint32_t framesInFlight = Renderer::GetConfiguration().FramesInFlight;
			arrayImageInfos.resize(framesInFlight);
			m_DescriptorsDirty = false;

			for (uint32_t frame = 0; frame < framesInFlight; ++frame)
			{
				m_WriteDescriptors[frame].clear();
				arrayImageInfos[frame].resize(numSets);
				for (uint32_t set = 0; set < numSets; ++set)
				{
					for (auto& pending : m_ImageDescriptors[set])
					{
						if (pending.Image.Raw())
						{
							pending.WriteDescriptor.pImageInfo = pending.Mip == -1 ? &pending.Image->GetDescriptor() 
																				   : &pending.Image->RT_GetMipImageDescriptor(pending.Mip);
							pending.WriteDescriptor.dstSet = m_DescriptorSets[frame][set];
							m_WriteDescriptors[frame].push_back(pending.WriteDescriptor);
						}
					}
					for (auto& pending : m_ImageArraysDescriptors[set])
					{
						if (pending.Images.empty())
							continue;

						for (auto& image : pending.Images)
						{
							arrayImageInfos[frame][set].push_back(image->GetDescriptor());
						}
						pending.WriteDescriptor.pImageInfo = arrayImageInfos[frame][set].data();
						pending.WriteDescriptor.descriptorCount = arrayImageInfos[frame][set].size();
						pending.WriteDescriptor.dstSet = m_DescriptorSets[frame][set];
						m_WriteDescriptors[frame].push_back(pending.WriteDescriptor);
					}

					if (!uniformBufferDescriptors.empty())
					{
						for (auto& desc : uniformBufferDescriptors[frame][set])
						{
							m_WriteDescriptors[frame].push_back(desc);
							auto& last = m_WriteDescriptors[frame].back();
							last.dstSet = m_DescriptorSets[frame][set];
						}
					}
					if (!storageBufferDescriptors.empty())
					{
						for (auto& desc : storageBufferDescriptors[frame][set])
						{
							m_WriteDescriptors[frame].push_back(desc);
							auto& last = m_WriteDescriptors[frame].back();
							last.dstSet = m_DescriptorSets[frame][set];
						}
					}
				}
			}
		}
		const uint32_t frameIndex = Renderer::GetAPIContext()->GetCurrentFrame();
		auto vulkanDevice = VulkanContext::GetCurrentDevice()->GetVulkanDevice();
		vkUpdateDescriptorSets(vulkanDevice, (uint32_t)m_WriteDescriptors[frameIndex].size(), m_WriteDescriptors[frameIndex].data(), 0, nullptr);
	}

	const ByteBuffer VulkanMaterial::GetFSUniformsBuffer() const
	{
		const size_t vertexBufferSize = m_Shader->GetVertexBufferSize();
		if (m_UniformsBuffer.Size - vertexBufferSize != 0)
			return ByteBuffer(&m_UniformsBuffer.Data[vertexBufferSize], m_UniformsBuffer.Size - vertexBufferSize);
		return ByteBuffer(nullptr, 0);
	}

	const ByteBuffer VulkanMaterial::GetVSUniformsBuffer() const
	{
		const size_t vertexBufferSize = m_Shader->GetVertexBufferSize();
		return ByteBuffer(m_UniformsBuffer.Data, vertexBufferSize);
	}

	void VulkanMaterial::tryAllocateDescriptorSets()
	{
		VulkanDescriptorAllocator::Version newVersion = VulkanRendererAPI::GetDescriptorAllocatorVersion();
		if (newVersion != m_DescriptorsVersion)
		{
			m_DescriptorsVersion = newVersion;
			Ref<VulkanShader> vulkanShader = m_Shader;
			const auto& shaderDescriptorSets = vulkanShader->GetDescriptorSets();
			for (auto& descriptorSet : m_DescriptorSets) // Per frame
			{
				for (uint32_t set = 0; set < descriptorSet.size(); ++set)
				{
					descriptorSet[set] = VulkanRendererAPI::RT_AllocateDescriptorSet(shaderDescriptorSets[set].DescriptorSetLayout);
				}
			}
		}
	}

	void VulkanMaterial::allocateStorage(const std::unordered_map<std::string, ShaderBuffer>& buffers, ByteBuffer& buffer) const
	{
		uint32_t size = 0;
		for (auto [name, shaderBuffer] : buffers)
			size += shaderBuffer.Size;

		if (buffer.Size != size)
		{
			if (buffer)
				buffer.Destroy();
			if (size)
			{
				buffer.Allocate(size);
				buffer.ZeroInitialize();
			}
		}
	}

	void VulkanMaterial::setDescriptor(const std::string& name, const Ref<Image2D>& image, int32_t mip)
	{
		const ShaderResourceDeclaration* resource = findResourceDeclaration(name);
		XYZ_ASSERT(resource, "");

		uint32_t binding = resource->GetRegister();
		// Texture is already set
		//if (binding < m_Images.size() && m_Images[binding].Raw() && image.Raw() == m_Images[binding].Raw())
		//	return;

		auto [wds, set] = m_Shader->GetDescriptorSet(name);
		VkWriteDescriptorSet textureDescriptor = *wds;
		uint32_t textureSet = set;

		Ref<VulkanMaterial> instance = this;
		Ref<VulkanImage2D> vulkanImage = image;
		Renderer::Submit([instance, vulkanImage, textureDescriptor, textureSet, binding, mip]() mutable {

			if (binding >= instance->m_Images.size())
				instance->m_Images.resize(static_cast<size_t>(binding) + 1);
			instance->m_Images[binding] = vulkanImage;

			if (instance->m_ImageDescriptors[textureSet].size() <= binding)
				instance->m_ImageDescriptors[textureSet].resize(static_cast<size_t>(binding) + 1);

			auto& desc = instance->m_ImageDescriptors[textureSet][binding];

			desc.Image = vulkanImage;
			desc.WriteDescriptor = textureDescriptor;
			desc.Mip = mip;
			instance->m_DescriptorsDirty = true;
		});
	}
	void VulkanMaterial::setDescriptor(const std::string& name, uint32_t index, const Ref<Image2D>& image)
	{
		const ShaderResourceDeclaration* resource = findResourceDeclaration(name);
		XYZ_ASSERT(resource, "");

		uint32_t binding = resource->GetRegister();
		// Texture is already set
		//if (binding < m_ImageArrays.size() && m_ImageArrays[binding].size() < index && image.Raw() == m_ImageArrays[binding][index].Raw())
		//	return;

		
		auto [wds, set] = m_Shader->GetDescriptorSet(name);
		VkWriteDescriptorSet textureDescriptor = *wds;
		uint32_t textureSet = set;

		Ref<VulkanMaterial> instance = this;
		Ref<VulkanImage2D> vulkanImage = image;
		Renderer::Submit([instance, vulkanImage, textureDescriptor, textureSet, binding, index]() mutable {

			if (binding >= instance->m_ImageArrays.size())
				instance->m_ImageArrays.resize(static_cast<size_t>(binding) + 1);
			if (index >= instance->m_ImageArrays[binding].size())
				instance->m_ImageArrays[binding].resize(static_cast<size_t>(index) + 1);

			instance->m_ImageArrays[binding][index] = vulkanImage;

			if (instance->m_ImageArraysDescriptors[textureSet].size() <= binding)
				instance->m_ImageArraysDescriptors[textureSet].resize(static_cast<size_t>(binding) + 1);

			if (instance->m_ImageArraysDescriptors[textureSet][binding].Images.size() <= index)
				instance->m_ImageArraysDescriptors[textureSet][binding].Images.resize(static_cast<size_t>(index) + 1);


			auto& desc = instance->m_ImageArraysDescriptors[textureSet][binding];
			desc.Images[index] = vulkanImage;
			desc.WriteDescriptor = textureDescriptor;

			instance->m_DescriptorsDirty = true;
		});
	}

	std::pair<const ShaderUniform*, ByteBuffer*> VulkanMaterial::findUniformDeclaration(const std::string& name)
	{
		std::string bufferName = Utils::FirstSubString(name, '.');
		const auto& shaderBuffers = m_Shader->GetBuffers();
		auto itBuffer = shaderBuffers.find(bufferName);
		if (itBuffer != shaderBuffers.end())
		{
			const ShaderBuffer& buffer = itBuffer->second;
			auto itUniform = buffer.Uniforms.find(name);
			if (itUniform != buffer.Uniforms.end())
				return { &itUniform->second, &m_UniformsBuffer };
		}
		return { nullptr, nullptr };
	}

	const ShaderResourceDeclaration* VulkanMaterial::findResourceDeclaration(const std::string& name)
	{
		auto& resources = m_Shader->GetResources();
		for (const auto& [n, resource] : resources)
		{
			if (resource.GetName() == name)
				return &resource;
		}
		return nullptr;
	}
}