#include "stdafx.h"
#include "VulkanMaterial.h"

#include "VulkanRendererAPI.h"
#include "VulkanTexture.h"

namespace XYZ {
	VulkanMaterial::VulkanMaterial(const Ref<Shader>& shader)
		:
		m_Shader(shader),
		m_WriteDescriptors(Renderer::GetConfiguration().FramesInFlight),
		m_DescriptorsDirty(true)
	{
		allocateStorage();
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
		m_DescriptorsDirty = true;
		Ref<VulkanShader> vulkanShader = m_Shader;
		const auto& shaderDescriptorSets = vulkanShader->GetDescriptorSets();
		m_TextureDescriptors.clear();
		m_DescriptorSets.resize(Renderer::GetConfiguration().FramesInFlight);
		m_TextureDescriptors.resize(shaderDescriptorSets.size());
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
		if (m_DescriptorsDirty)
		{
			Ref<VulkanShader> vulkanShader = m_Shader;
			const uint32_t numSets = vulkanShader->GetDescriptorSets().size();
			m_DescriptorsDirty = false;
			for (uint32_t frame = 0; frame < Renderer::GetConfiguration().FramesInFlight; ++frame)
			{
				m_WriteDescriptors[frame].clear();
				
				for (uint32_t set = 0; set < numSets; ++set)
				{
					for (auto& pending : m_TextureDescriptors[set])
					{
						m_WriteDescriptors[frame].push_back(pending);
						auto& last = m_WriteDescriptors[frame].back();
						last.dstSet = m_DescriptorSets[frame][set];
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
	void VulkanMaterial::allocateStorage()
	{
		const auto& shaderBuffers = m_Shader->GetBuffers();
		if (shaderBuffers.size() > 0)
		{
			uint32_t size = 0;
			for (auto [name, shaderBuffer] : shaderBuffers)
				size += shaderBuffer.Size;

			m_UniformsBuffer.Allocate(size);
			m_UniformsBuffer.ZeroInitialize();
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

		auto [wds, set] = m_Shader.As<VulkanShader>()->GetDescriptorSet(name);
		VkWriteDescriptorSet textureDescriptor = *wds;
		uint32_t textureSet = set;

		Ref<VulkanMaterial> instance = this;
		Renderer::Submit([instance, textureDescriptor, textureSet, binding]() mutable {
			instance->m_TextureDescriptors[textureSet].push_back(textureDescriptor);
			auto& writeDesc = instance->m_TextureDescriptors[textureSet].back();
			writeDesc.pImageInfo = &instance->m_Textures[binding].As<VulkanTexture2D>()->GetVulkanDescriptorInfo();
		});
		
		m_DescriptorsDirty = true;
	}

	const ShaderUniform* VulkanMaterial::findUniformDeclaration(const std::string& name)
	{
		const auto& shaderBuffers = m_Shader->GetBuffers();

		XYZ_ASSERT(shaderBuffers.size() <= 1, "We currently only support ONE material buffer!");

		if (shaderBuffers.size() > 0)
		{
			const ShaderBuffer& buffer = (*shaderBuffers.begin()).second;
			if (buffer.Uniforms.find(name) == buffer.Uniforms.end())
				return nullptr;

			return &buffer.Uniforms.at(name);
		}
		return nullptr;
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