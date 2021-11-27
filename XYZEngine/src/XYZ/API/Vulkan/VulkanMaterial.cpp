#include "stdafx.h"
#include "VulkanMaterial.h"

#include "VulkanRendererAPI.h"

#include "XYZ/Utils/StringUtils.h"

namespace XYZ {
	VulkanMaterial::VulkanMaterial(const Ref<Shader>& shader)
		:
		m_Shader(shader),
		m_WriteDescriptors(Renderer::GetConfiguration().FramesInFlight),
		m_DescriptorsDirty(true)
	{
		allocateStorages();
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
	}
	void VulkanMaterial::Invalidate()
	{
		allocateStorages();
		m_DescriptorsDirty = true;
		Ref<VulkanShader> vulkanShader = m_Shader;
		const auto& shaderDescriptorSets = vulkanShader->GetDescriptorSets();
		m_DescriptorSets.resize(Renderer::GetConfiguration().FramesInFlight);
		
		m_TextureDescriptors.clear();
		m_TextureArraysDescriptors.clear();
		m_TextureDescriptors.resize(shaderDescriptorSets.size());
		m_TextureArraysDescriptors.resize(shaderDescriptorSets.size());

		for (auto& descriptorSet : m_DescriptorSets) // Per frame
		{
			descriptorSet.resize(shaderDescriptorSets.size());
			for (uint32_t set = 0; set < descriptorSet.size(); ++set)
			{
				descriptorSet[set] = VulkanRendererAPI::RT_AllocateDescriptorSet(shaderDescriptorSets[set].DescriptorSetLayout);
			}
		}
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
	void VulkanMaterial::Set(const std::string& name, const Ref<Texture2D>& texture)
	{
		setDescriptor(name, texture);
	}
	void VulkanMaterial::Set(const std::string& name, const Ref<Texture2D>& texture, uint32_t arrayIndex)
	{
		setDescriptor(name, texture, arrayIndex);
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
	void VulkanMaterial::RT_UpdateForRendering(const vector3D<VkWriteDescriptorSet>& descriptors)
	{
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
					for (auto& pending : m_TextureDescriptors[set])
					{
						if (pending.Texture.Raw())
						{
							pending.WriteDescriptor.pImageInfo = &pending.Texture->GetVulkanDescriptorInfo();
							pending.WriteDescriptor.dstSet = m_DescriptorSets[frame][set];
							m_WriteDescriptors[frame].push_back(pending.WriteDescriptor);
						}
					}	
					for (auto& pending : m_TextureArraysDescriptors[set])
					{
						if (pending.Textures.empty())
							continue;

						for (auto& texture : pending.Textures)
						{
							arrayImageInfos[frame][set].push_back(texture->GetVulkanDescriptorInfo());
						}
						pending.WriteDescriptor.pImageInfo = arrayImageInfos[frame][set].data();
						pending.WriteDescriptor.descriptorCount = arrayImageInfos[frame][set].size();
						pending.WriteDescriptor.dstSet = m_DescriptorSets[frame][set];
						m_WriteDescriptors[frame].push_back(pending.WriteDescriptor);
					}

					for (auto& desc : descriptors[frame][set])
					{
						m_WriteDescriptors[frame].push_back(desc);
						auto& last = m_WriteDescriptors[frame].back();
						last.dstSet = m_DescriptorSets[frame][set];
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

	void VulkanMaterial::allocateStorages()
	{
		allocateStorage(m_Shader->GetBuffers(), m_UniformsBuffer);
	}
	void VulkanMaterial::allocateStorage(const std::unordered_map<std::string, ShaderBuffer>& buffers, ByteBuffer& buffer)
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
	void VulkanMaterial::setDescriptor(const std::string& name, const Ref<Texture2D>& texture)
	{
		const ShaderResourceDeclaration* resource = findResourceDeclaration(name);
		XYZ_ASSERT(resource, "");

		uint32_t binding = resource->GetRegister();
		// Texture is already set
		if (binding < m_Textures.size() && m_Textures[binding].Raw() && texture.Raw() == m_Textures[binding].Raw())
			return;

		if (binding >= m_Textures.size())
			m_Textures.resize(static_cast<size_t>(binding) + 1);
		m_Textures[binding] = texture;

		auto [wds, set] = m_Shader->GetDescriptorSet(name);
		VkWriteDescriptorSet textureDescriptor = *wds;
		uint32_t textureSet = set;

		Ref<VulkanMaterial> instance = this;
		Renderer::Submit([instance, textureDescriptor, textureSet, binding]() mutable {
	
			Ref<VulkanTexture2D> vulkanTexture = instance->m_Textures[binding];
			if (instance->m_TextureDescriptors[textureSet].size() <= binding)
				instance->m_TextureDescriptors[textureSet].resize(static_cast<size_t>(binding) + 1);

			instance->m_TextureDescriptors[textureSet][binding].Texture = vulkanTexture;
			instance->m_TextureDescriptors[textureSet][binding].WriteDescriptor = textureDescriptor;
			instance->m_DescriptorsDirty = true;
		});
	}

	void VulkanMaterial::setDescriptor(const std::string& name, const Ref<Texture2D>& texture, uint32_t index)
	{
		const ShaderResourceDeclaration* resource = findResourceDeclaration(name);
		XYZ_ASSERT(resource, "");
	
		uint32_t binding = resource->GetRegister();
		// Texture is already set
		if (binding < m_TextureArrays.size() && m_TextureArrays[binding].size() < index && texture.Raw() == m_TextureArrays[binding][index].Raw())
			return;

		if (binding >= m_TextureArrays.size())
			m_TextureArrays.resize(static_cast<size_t>(binding) + 1);
		if (index >= m_TextureArrays[binding].size())
			m_TextureArrays[binding].resize(static_cast<size_t>(index) + 1);

		m_TextureArrays[binding][index] = texture;
		auto [wds, set] = m_Shader->GetDescriptorSet(name);
		VkWriteDescriptorSet textureDescriptor = *wds;
		uint32_t textureSet = set;

		Ref<VulkanMaterial> instance = this;
		Renderer::Submit([instance, textureDescriptor, textureSet, binding, index]() mutable {
			
			Ref<VulkanTexture2D> vulkanTexture = instance->m_TextureArrays[binding][index];

			if (instance->m_TextureArraysDescriptors[textureSet].size() <= binding)
				instance->m_TextureArraysDescriptors[textureSet].resize(static_cast<size_t>(binding) + 1);
			
			if (instance->m_TextureArraysDescriptors[textureSet][binding].Textures.size() <= index)
				instance->m_TextureArraysDescriptors[textureSet][binding].Textures.resize(static_cast<size_t>(index) + 1);

			instance->m_TextureArraysDescriptors[textureSet][binding].Textures[index] = vulkanTexture;
			instance->m_TextureArraysDescriptors[textureSet][binding].WriteDescriptor = textureDescriptor;
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