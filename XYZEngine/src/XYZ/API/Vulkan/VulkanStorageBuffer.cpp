#include "stdafx.h"
#include "VulkanStorageBuffer.h"

#include "VulkanContext.h"

namespace XYZ {
	VulkanStorageBuffer::VulkanStorageBuffer(uint32_t size, uint32_t binding)
		: m_Size(size), m_Binding(binding)
	{
		Ref<VulkanStorageBuffer> instance = this;
		Renderer::Submit([instance]() mutable
		{
			instance->RT_invalidate();
		});
	}
	VulkanStorageBuffer::VulkanStorageBuffer(const void* data, uint32_t size, uint32_t binding)
	{
		ByteBuffer buffer;
		if (m_Buffers.Empty())
			buffer.Allocate(m_Size);
		else
			buffer = m_Buffers.PopBack();

		buffer.Write(data, size);

		Ref<VulkanStorageBuffer> instance = this;
		Renderer::Submit([instance, buffer]() mutable
		{
			instance->RT_invalidate();
			instance->RT_Update(buffer.Data, buffer.Size, 0);
			instance->m_Buffers.PushBack(buffer);
		});
	}
	VulkanStorageBuffer::~VulkanStorageBuffer()
	{
		release();
	}
	void VulkanStorageBuffer::Update(const void* data, uint32_t size, uint32_t offset)
	{
		ByteBuffer buffer;
		if (m_Buffers.Empty())
			buffer.Allocate(m_Size);
		else
			buffer = m_Buffers.PopBack();

		buffer.Write(data, size, offset);
		Ref<VulkanStorageBuffer> instance = this;
		Renderer::Submit([instance, size, offset, buffer]() mutable {
			instance->RT_Update(buffer.Data, size, offset);
			instance->m_Buffers.PushBack(buffer);
		});
	}
	void VulkanStorageBuffer::release()
	{
		if (!m_MemoryAllocation)
			return;

		Renderer::SubmitResource([buffer = m_Buffer, memoryAlloc = m_MemoryAllocation]()
		{
			VulkanAllocator allocator("StorageBuffer");
			allocator.DestroyBuffer(buffer, memoryAlloc);
		});

		m_Buffer = nullptr;
		m_MemoryAllocation = nullptr;
		while (!m_Buffers.Empty())
		{
			auto buffer = m_Buffers.PopBack();
			buffer.Destroy();
		}
	}
	void VulkanStorageBuffer::RT_invalidate()
	{
		release();

		VkBufferCreateInfo bufferInfo = {};
		bufferInfo.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
		bufferInfo.usage = VK_BUFFER_USAGE_STORAGE_BUFFER_BIT;
		bufferInfo.size = m_Size;

		VulkanAllocator allocator("StorageBuffer");
		m_MemoryAllocation = allocator.AllocateBuffer(bufferInfo, VMA_MEMORY_USAGE_GPU_ONLY, m_Buffer);

		m_DescriptorInfo.buffer = m_Buffer;
		m_DescriptorInfo.offset = 0;
		m_DescriptorInfo.range = m_Size;
	}
	void VulkanStorageBuffer::RT_Update(const void* data, uint32_t size, uint32_t offset)
	{
		VulkanAllocator allocator("VulkanStorageBuffer");
		uint8_t* pData = allocator.MapMemory<uint8_t>(m_MemoryAllocation);
		memcpy(pData, (uint8_t*)data + offset, size);
		allocator.UnmapMemory(m_MemoryAllocation);
	}
}