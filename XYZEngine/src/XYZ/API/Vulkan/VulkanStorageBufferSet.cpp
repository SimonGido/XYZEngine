#include "stdafx.h"
#include "VulkanStorageBufferSet.h"

#include "VulkanShader.h"

namespace XYZ {
	VulkanStorageBufferSet::VulkanStorageBufferSet(uint32_t frames)
		:
		m_Frames(frames)
	{
		m_IndirectBuffers.resize(frames);
	}
	VulkanStorageBufferSet::~VulkanStorageBufferSet()
	{
		for (auto &[key, set] :m_DataTransferBuffers)
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
	void VulkanStorageBufferSet::Update(const void* data, uint32_t size, uint32_t offset, uint32_t binding, uint32_t set)
	{
		// TODO: queue for buffers
		Ref<VulkanStorageBufferSet> instance = this;
		ByteBuffer buffer = getBuffer(set, binding, size);
		buffer.Write(data, size);

		Renderer::Submit([buffer, instance, size, offset, binding, set]() mutable {
			const uint32_t frame = Renderer::GetCurrentFrame();
			instance->Get(binding, set, frame)->RT_Update(buffer.Data, size, offset);
			instance->m_DataTransferBuffers[set][binding].PushBack(buffer);
		});
	}
	void VulkanStorageBufferSet::Update(void** data, uint32_t size, uint32_t offset, uint32_t binding, uint32_t set)
	{
		Ref<VulkanStorageBufferSet> instance = this;
		void* dataPtr = *data;
		data = nullptr;

		Renderer::Submit([dataPtr, instance, size, offset, binding, set]() mutable {
			const uint32_t frame = Renderer::GetCurrentFrame();
			instance->Get(binding, set, frame)->RT_Update(dataPtr, size, offset);
			delete[] dataPtr;
		});
	}
	void VulkanStorageBufferSet::UpdateEachFrame(const void* data, uint32_t size, uint32_t offset, uint32_t binding, uint32_t set)
	{
		Ref<VulkanStorageBufferSet> instance = this;
		
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

	void VulkanStorageBufferSet::CreateDescriptors(const Ref<Shader>& shader)
	{
		Ref<const VulkanShader> vulkanShader = shader;

		Ref<VulkanStorageBufferSet> instance = this;
		Renderer::Submit([instance, vulkanShader]() mutable {
			instance->RT_createDescriptors(vulkanShader);
		});
	}
	void VulkanStorageBufferSet::Create(uint32_t size, uint32_t set, uint32_t binding, bool indirect)
	{
		for (uint32_t frame = 0; frame < m_Frames; frame++)
		{
			Ref<StorageBuffer> storageBuffer = StorageBuffer::Create(size, binding, indirect);
			Set(storageBuffer, set, frame);
		}
	}
	void VulkanStorageBufferSet::Set(Ref<StorageBuffer> storageBuffer, uint32_t set, uint32_t frame)
	{
		if (storageBuffer->IsIndirect())
			m_IndirectBuffers[frame].push_back(storageBuffer.As<VulkanStorageBuffer>());

		m_StorageBuffers[frame][set][storageBuffer->GetBinding()] = storageBuffer.As<VulkanStorageBuffer>();
	}
	void VulkanStorageBufferSet::Resize(uint32_t size, uint32_t set, uint32_t binding)
	{
		for (uint32_t frame = 0; frame < m_Frames; frame++)
		{
			m_StorageBuffers.at(frame).at(set).at(binding)->Resize(size);
		}
	}
	void VulkanStorageBufferSet::SetBufferInfo(uint32_t size, uint32_t offset, uint32_t binding, uint32_t set)
	{
		Ref<VulkanStorageBufferSet> instance = this;

		Renderer::Submit([instance, size, offset, binding, set]() mutable {
			const uint32_t frame = Renderer::GetCurrentFrame();
			instance->Get(binding, set, frame).As<VulkanStorageBuffer>()->RT_SetBufferInfo(size, offset);
		});
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
	const std::vector<std::vector<std::vector<VkWriteDescriptorSet>>>& VulkanStorageBufferSet::GetDescriptors(const Ref<Shader>& shader)
	{
		size_t hash = shader->GetHash();
		auto it = m_WriteDescriptors.find(hash);
		if (it != m_WriteDescriptors.end())
			return it->second;

		RT_createDescriptors(shader);
		return GetDescriptors(shader);
	}
	void VulkanStorageBufferSet::RT_createDescriptors(const Ref<VulkanShader>& shader)
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

					writeDescr.pBufferInfo = &m_StorageBuffers[frame][set][binding]->GetDescriptorBufferInfo();
					
					writeDescr.pImageInfo = nullptr; // Optional
					writeDescr.pTexelBufferView = nullptr; // Optional
				}
			}
		}
	}
	ByteBuffer VulkanStorageBufferSet::getBuffer(uint32_t set, uint32_t binding, uint32_t size)
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