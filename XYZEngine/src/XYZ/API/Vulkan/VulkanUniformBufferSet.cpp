#include "stdafx.h"
#include "VulkanUniformBufferSet.h"

#include "VulkanShader.h"
#include "VulkanRendererAPI.h"

namespace XYZ {
	VulkanUniformBufferSet::VulkanUniformBufferSet(uint32_t frames)
		:
		m_Frames(frames)
	{
	}
	void VulkanUniformBufferSet::CreateDescriptors(const Ref<Shader>& shader)
	{
		Ref<const VulkanShader> vulkanShader = shader;

		Ref<VulkanUniformBufferSet> instance = this;
		Renderer::Submit([instance, vulkanShader]() mutable {
			auto it = instance->m_WriteDescriptors.find(vulkanShader->GetHash());
			if (it != instance->m_WriteDescriptors.end())
				it->second.clear();

			auto& writeDescriptor = instance->m_WriteDescriptors[vulkanShader->GetHash()];
			const auto& descriptorSets = vulkanShader->GetDescriptorSets();
			writeDescriptor.resize(instance->m_Frames);
			for (uint32_t frame = 0; frame < instance->m_Frames; ++frame)
			{
				writeDescriptor[frame].resize(descriptorSets.size());
				for (size_t set = 0; set < descriptorSets.size(); ++set)
				{
					for (auto&& [binding, shaderUB] : descriptorSets[set].ShaderDescriptorSet.UniformBuffers)
					{
						// Create new write description for each uniform buffer
						auto& writeDescr = writeDescriptor[frame][set].emplace_back();

						writeDescr.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
						writeDescr.pNext = nullptr;
						writeDescr.dstSet = VK_NULL_HANDLE;
						writeDescr.dstBinding = binding;
						writeDescr.dstArrayElement = 0;
						writeDescr.descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
						writeDescr.descriptorCount = 1;

						writeDescr.pBufferInfo = &instance->m_UniformBuffers[frame][set][binding]->GetDescriptorBufferInfo();
						writeDescr.pImageInfo = nullptr; // Optional
						writeDescr.pTexelBufferView = nullptr; // Optional
					}
				}
			}
		});	
	}
	void VulkanUniformBufferSet::Create(uint32_t size, uint32_t set, uint32_t binding)
	{
		for (uint32_t frame = 0; frame < m_Frames; frame++)
		{
			Ref<UniformBuffer> uniformBuffer = UniformBuffer::Create(size, binding);
			Set(uniformBuffer, set, frame);
		}
	}
	void VulkanUniformBufferSet::Set(Ref<UniformBuffer> uniformBuffer, uint32_t set, uint32_t frame)
	{
		m_UniformBuffers[frame][set][uniformBuffer->GetBinding()] = uniformBuffer.As<VulkanUniformBuffer>();
	}
	Ref<UniformBuffer> VulkanUniformBufferSet::Get(uint32_t binding, uint32_t set, uint32_t frame)
	{
		XYZ_ASSERT(m_UniformBuffers.find(frame) != m_UniformBuffers.end(), "");
		XYZ_ASSERT(m_UniformBuffers.at(frame).find(set) != m_UniformBuffers.at(frame).end(), "");
		XYZ_ASSERT(m_UniformBuffers.at(frame).at(set).find(binding) != m_UniformBuffers.at(frame).at(set).end(), "");

		return m_UniformBuffers.at(frame).at(set).at(binding);
	}
	
	const std::vector<std::vector<std::vector<VkWriteDescriptorSet>>>& VulkanUniformBufferSet::GetDescriptors(size_t hash) const
	{
		XYZ_ASSERT(m_WriteDescriptors.find(hash) != m_WriteDescriptors.end(), "UniformBufferSet does not have descriptors");
		return m_WriteDescriptors.at(hash);
	}
}
