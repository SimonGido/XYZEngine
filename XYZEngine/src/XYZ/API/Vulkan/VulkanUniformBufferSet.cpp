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
	VulkanUniformBufferSet::~VulkanUniformBufferSet()
	{
		for (auto& [key, set] : m_DataTransferBuffers)
		{
			for (auto& [binding, queue] : set)
			{
				while (!queue.Empty())
				{
					ByteBuffer buffer = queue.PopBack();
					buffer.Destroy();
				}
			}
		}
	}
	void VulkanUniformBufferSet::UpdateEachFrame(const void* data, uint32_t size, uint32_t offset, uint32_t binding, uint32_t set)
	{
		Ref<VulkanUniformBufferSet> instance = this;
		ByteBuffer buffer = getBuffer(set, binding, size);
		buffer.Write(data, size);

		Renderer::Submit([buffer, instance, size, offset, binding, set]() mutable {
			for (uint32_t frame = 0; frame < Renderer::GetConfiguration().FramesInFlight; ++frame)
			{
				instance->Get(binding, set, frame)->RT_Update(buffer, size, offset);
			}
			instance->m_DataTransferBuffers[set][binding].PushBack(buffer);
		});
	}
	void VulkanUniformBufferSet::CreateDescriptors(const Ref<Shader>& shader)
	{
		Ref<const VulkanShader> vulkanShader = shader;

		Ref<VulkanUniformBufferSet> instance = this;
		Renderer::Submit([instance, vulkanShader]() mutable {
			instance->RT_createDescriptors(vulkanShader);
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

	bool VulkanUniformBufferSet::HasDescriptors(size_t hash) const
	{
		return m_WriteDescriptors.find(hash) != m_WriteDescriptors.end();
	}
	void VulkanUniformBufferSet::SetBufferInfo(uint32_t size, uint32_t offset, uint32_t binding, uint32_t set)
	{
		Ref<VulkanUniformBufferSet> instance = this;
		Renderer::Submit([instance, binding, set, size, offset]() mutable {
			instance->Get(binding, set, Renderer::GetCurrentFrame()).As<VulkanUniformBuffer>()->RT_SetBufferInfo(size, offset);
		});
	}

	const std::vector<std::vector<std::vector<VkWriteDescriptorSet>>>& VulkanUniformBufferSet::GetDescriptors(size_t hash) const
	{
		XYZ_ASSERT(m_WriteDescriptors.find(hash) != m_WriteDescriptors.end(), "UniformBufferSet does not have descriptors");
		return m_WriteDescriptors.at(hash);
	}
	const std::vector<std::vector<std::vector<VkWriteDescriptorSet>>>& VulkanUniformBufferSet::GetDescriptors(const Ref<Shader>& shader)
	{
		size_t hash = shader->GetHash();
		auto it = m_WriteDescriptors.find(hash);
		if (it != m_WriteDescriptors.end())
			return it->second;
		
		RT_createDescriptors(shader);
		return GetDescriptors(shader);
	}
	void VulkanUniformBufferSet::RT_createDescriptors(const Ref<VulkanShader>& shader)
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

					writeDescr.pBufferInfo = &m_UniformBuffers[frame][set][binding]->GetDescriptorBufferInfo();
					writeDescr.pImageInfo = nullptr; // Optional
					writeDescr.pTexelBufferView = nullptr; // Optional
				}
			}
		}
	}
	ByteBuffer VulkanUniformBufferSet::getBuffer(uint32_t set, uint32_t binding, uint32_t size)
	{
		auto& queue = m_DataTransferBuffers[set][binding];

		ByteBuffer buffer;
		if (queue.Empty())
			buffer.Allocate(size);
		else
		{
			buffer = queue.PopBack();
			buffer.TryReallocate(size);
		}
		return buffer;
	}
}
