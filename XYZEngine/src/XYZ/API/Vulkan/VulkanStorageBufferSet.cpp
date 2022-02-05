#include "stdafx.h"
#include "VulkanStorageBufferSet.h"

#include "VulkanShader.h"

namespace XYZ {
	VulkanStorageBufferSet::VulkanStorageBufferSet(uint32_t frames)
		:
		m_Frames(frames)
	{
	}
	void VulkanStorageBufferSet::CreateDescriptors(const Ref<Shader>& shader)
	{
		Ref<const VulkanShader> vulkanShader = shader;

		Ref<VulkanStorageBufferSet> instance = this;
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
					for (auto&& [binding, shaderUB] : descriptorSets[set].ShaderDescriptorSet.StorageBuffers)
					{
						// Create new write description for each uniform buffer
						auto& writeDescr = writeDescriptor[frame][set].emplace_back();

						writeDescr.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
						writeDescr.pNext = nullptr;
						writeDescr.dstSet = VK_NULL_HANDLE;
						writeDescr.dstBinding = binding;
						writeDescr.dstArrayElement = 0;
						writeDescr.descriptorType = VK_DESCRIPTOR_TYPE_STORAGE_BUFFER;
						writeDescr.descriptorCount = 1;

						writeDescr.pBufferInfo = &instance->m_StorageBuffers[frame][set][binding]->GetDescriptorBufferInfo();
						writeDescr.pImageInfo = nullptr; // Optional
						writeDescr.pTexelBufferView = nullptr; // Optional
					}
				}
			}
		});
	}
	void VulkanStorageBufferSet::Create(uint32_t size, uint32_t set, uint32_t binding)
	{
		for (uint32_t frame = 0; frame < m_Frames; frame++)
		{
			Ref<StorageBuffer> storageBuffer = StorageBuffer::Create(size, binding);
			Set(storageBuffer, set, frame);
		}
	}
	void VulkanStorageBufferSet::Set(Ref<StorageBuffer> storageBuffer, uint32_t set, uint32_t frame)
	{
		m_StorageBuffers[frame][set][storageBuffer->GetBinding()] = storageBuffer.As<VulkanStorageBuffer>();
	}
	Ref<StorageBuffer> VulkanStorageBufferSet::Get(uint32_t binding, uint32_t set, uint32_t frame)
	{
		XYZ_ASSERT(m_StorageBuffers.find(frame) != m_StorageBuffers.end(), "");
		XYZ_ASSERT(m_StorageBuffers.at(frame).find(set) != m_StorageBuffers.at(frame).end(), "");
		XYZ_ASSERT(m_StorageBuffers.at(frame).at(set).find(binding) != m_StorageBuffers.at(frame).at(set).end(), "");

		return m_StorageBuffers.at(frame).at(set).at(binding);
	}
	const std::vector<std::vector<std::vector<VkWriteDescriptorSet>>>& VulkanStorageBufferSet::GetDescriptors(size_t hash) const
	{
		XYZ_ASSERT(m_WriteDescriptors.find(hash) != m_WriteDescriptors.end(), "StorageBufferSet does not have descriptors");
		return m_WriteDescriptors.at(hash);
	}
}