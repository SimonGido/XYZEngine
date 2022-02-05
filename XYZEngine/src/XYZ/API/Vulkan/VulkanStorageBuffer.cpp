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
		:
		m_Size(size), m_Binding(binding)
	{
		ByteBuffer buffer = GetBuffer();
		buffer.Write(data, size);

		Ref<VulkanStorageBuffer> instance = this;
		Renderer::Submit([instance, buf = std::move(buffer)]() mutable
		{
			instance->RT_invalidate();
			instance->RT_Update(buf.Data, buf.Size, 0);
			instance->m_Buffers.PushBack(buf);
		});
	}
	VulkanStorageBuffer::~VulkanStorageBuffer()
	{
		release();
	}
	void VulkanStorageBuffer::Update(const void* data, uint32_t size, uint32_t offset)
	{
		XYZ_ASSERT(size + offset <= m_Size, "");
		ByteBuffer buffer = GetBuffer();

		buffer.Write(data, size, offset);
		Ref<VulkanStorageBuffer> instance = this;
		Renderer::Submit([instance, size, offset, buffer]() mutable {
			instance->RT_Update(buffer.Data, size, offset);
			instance->m_Buffers.PushBack(buffer);
		});
	}
	void VulkanStorageBuffer::RT_Update(const void* data, uint32_t size, uint32_t offset)
	{
		VulkanAllocator allocator("VulkanStorageBuffer");
		uint8_t* pData = allocator.MapMemory<uint8_t>(m_MemoryAllocation);
		memcpy(pData, (uint8_t*)data + offset, size);
		allocator.UnmapMemory(m_MemoryAllocation);
	}
	void VulkanStorageBuffer::Update(ByteBuffer data, uint32_t size, uint32_t offset)
	{
		XYZ_ASSERT(data.Size <= m_Size, "");
		Ref<VulkanStorageBuffer> instance = this;
		Renderer::Submit([instance, data, size, offset]() mutable {
			instance->RT_Update(data.Data, size, offset);
			instance->m_Buffers.PushBack(data);
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
		m_MemoryAllocation = allocator.AllocateBuffer(bufferInfo, VMA_MEMORY_USAGE_CPU_TO_GPU, m_Buffer);

		m_DescriptorInfo.buffer = m_Buffer;
		m_DescriptorInfo.offset = 0;
		m_DescriptorInfo.range = m_Size;
	}
	ByteBuffer VulkanStorageBuffer::GetBuffer()
	{
		ByteBuffer buffer;
		if (m_Buffers.Empty())
			buffer.Allocate(m_Size);
		else
			buffer = m_Buffers.PopBack();
		return buffer;
	}

}