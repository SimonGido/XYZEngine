#include "stdafx.h"
#include "VulkanIndirectBufferSet.h"


namespace XYZ {
	VulkanIndirectBufferSet::VulkanIndirectBufferSet(uint32_t frames)
		:
		m_Frames(frames)
	{
	}
	
	void VulkanIndirectBufferSet::CreateDescriptors(const Ref<Shader>& shader)
	{
		Ref<const VulkanShader> vulkanShader = shader;

		Ref<VulkanIndirectBufferSet> instance = this;
		Renderer::Submit([instance, vulkanShader]() mutable {
			instance->RT_createDescriptors(vulkanShader);
			});
	}
	void VulkanIndirectBufferSet::Create(uint32_t size, uint32_t set, uint32_t binding)
	{
		for (uint32_t frame = 0; frame < m_Frames; frame++)
		{
			Ref<IndirectBuffer> indirectBuffer = IndirectBuffer::Create(nullptr, size, binding);
			Set(indirectBuffer, set, frame);
		}
	}
	void VulkanIndirectBufferSet::Set(Ref<IndirectBuffer> indirectBuffer, uint32_t set, uint32_t frame)
	{
		m_IndirectBuffers[frame][set][indirectBuffer->GetBinding()] = indirectBuffer.As<VulkanIndirectBuffer>();
	}

	Ref<IndirectBuffer> VulkanIndirectBufferSet::Get(uint32_t binding, uint32_t set, uint32_t frame)
	{
		XYZ_ASSERT(m_IndirectBuffers.find(frame) != m_IndirectBuffers.end(), "");
		XYZ_ASSERT(m_IndirectBuffers.at(frame).find(set) != m_IndirectBuffers.at(frame).end(), "");
		XYZ_ASSERT(m_IndirectBuffers.at(frame).at(set).find(binding) != m_IndirectBuffers.at(frame).at(set).end(), "");

		return m_IndirectBuffers.at(frame).at(set).at(binding);
	}
	const std::vector<std::vector<std::vector<VkWriteDescriptorSet>>>& VulkanIndirectBufferSet::GetDescriptors(size_t hash) const
	{
		XYZ_ASSERT(m_WriteDescriptors.find(hash) != m_WriteDescriptors.end(), "StorageBufferSet does not have descriptors");
		return m_WriteDescriptors.at(hash);
	}
	const std::vector<std::vector<std::vector<VkWriteDescriptorSet>>>& VulkanIndirectBufferSet::GetDescriptors(const Ref<Shader>& shader)
	{
		size_t hash = shader->GetHash();
		auto it = m_WriteDescriptors.find(hash);
		if (it != m_WriteDescriptors.end())
			return it->second;

		RT_createDescriptors(shader);
		return GetDescriptors(shader);
	}
	void VulkanIndirectBufferSet::RT_createDescriptors(const Ref<VulkanShader>& shader)
	{
		auto it = m_WriteDescriptors.find(shader->GetHash());
		if (it != m_WriteDescriptors.end())
			it->second.clear();

		auto& writeDescriptor = m_WriteDescriptors[shader->GetHash()];
		const auto& descriptorSets = shader->GetDescriptorSets();
		writeDescriptor.resize(m_Frames);
		for (uint32_t frame = 0; frame < m_Frames; ++frame)
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

					writeDescr.pBufferInfo = &m_IndirectBuffers[frame][set][binding]->GetDescriptorBufferInfo();
					writeDescr.pImageInfo = nullptr; // Optional
					writeDescr.pTexelBufferView = nullptr; // Optional
				}
			}
		}
	}
}