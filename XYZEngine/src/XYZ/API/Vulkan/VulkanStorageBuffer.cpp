#include "stdafx.h"
#include "VulkanStorageBuffer.h"

#include "VulkanContext.h"

namespace XYZ {

	static constexpr uint32_t sc_SizeLimit = 224395264u;

	VulkanStorageBuffer::VulkanStorageBuffer(uint32_t size, uint32_t binding, bool indirect)
		: 
		m_Size(size), 
		m_Binding(binding),
		m_IsIndirect(indirect)
	{
		Ref<VulkanStorageBuffer> instance = this;
		Renderer::Submit([instance]() mutable
		{
			instance->RT_invalidate();
		});
	}
	VulkanStorageBuffer::VulkanStorageBuffer(const void* data, uint32_t size, uint32_t binding, bool indirect)
		:
		m_Size(size), 
		m_Binding(binding),
		m_IsIndirect(indirect)
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
		if (size == 0)
			return;
		ByteBuffer buffer = GetBuffer();

		buffer.Write(data, size);
		Ref<VulkanStorageBuffer> instance = this;
		Renderer::Submit([instance, size, offset, buffer]() mutable {

			instance->RT_Update(buffer.Data, size, offset);
			instance->m_Buffers.PushBack(buffer);
		});
	}
	void VulkanStorageBuffer::Update(void** data, uint32_t size, uint32_t offset)
	{
		XYZ_ASSERT(size + offset <= m_Size, "");
		if (size == 0)
			return;

		void* dataPtr = *data;
		data = nullptr;

		Ref<VulkanStorageBuffer> instance = this;
		Renderer::Submit([instance, size, offset, dataPtr]() mutable {
			instance->RT_Update(dataPtr, size, offset);
			delete[]dataPtr;
		});
	}
	void VulkanStorageBuffer::RT_Update(const void* data, uint32_t size, uint32_t offset)
	{
		XYZ_PROFILE_FUNC("VulkanStorageBufferSet::RT_Update");
		//RT_ApplySize();
		XYZ_ASSERT(size + offset <= m_Size, "");
		if (size == 0)
			return;

		VulkanAllocator allocator("VulkanBuffer");
		if (m_Size >= sc_SizeLimit)
		{
			auto device = VulkanContext::GetCurrentDevice();
			VkBuffer stagingBuffer;
			VkBufferCreateInfo stagingBufferCreateInfo{};
			stagingBufferCreateInfo.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
			stagingBufferCreateInfo.size = size;
			stagingBufferCreateInfo.usage = VK_BUFFER_USAGE_TRANSFER_SRC_BIT;
			stagingBufferCreateInfo.sharingMode = VK_SHARING_MODE_EXCLUSIVE;
			const VmaAllocation stagingBufferAllocation = allocator.AllocateBuffer(stagingBufferCreateInfo, VMA_MEMORY_USAGE_CPU_ONLY, stagingBuffer);

			uint8_t* destData = allocator.MapMemory<uint8_t>(stagingBufferAllocation);
			memcpy(destData, data, size);
			allocator.UnmapMemory(stagingBufferAllocation);

			const VkCommandBuffer copyCmd = device->GetCommandBuffer(true);

			VkBufferCopy copyRegion = {};
			copyRegion.dstOffset = offset;
			copyRegion.size = size;
			vkCmdCopyBuffer(copyCmd, stagingBuffer, m_Buffer, 1, &copyRegion);

			device->FlushCommandBuffer(copyCmd);
			allocator.DestroyBuffer(stagingBuffer, stagingBufferAllocation);
		}
		else
		{
			uint8_t* pData = allocator.MapMemory<uint8_t>(m_MemoryAllocation);
			memcpy(pData + offset, (uint8_t*)data, size);
			allocator.UnmapMemory(m_MemoryAllocation);
		}
	}

	void VulkanStorageBuffer::Update(ByteBuffer data, uint32_t size, uint32_t offset)
	{
		XYZ_ASSERT(data.Size + offset <= m_Size, "");
		if (size == 0)
			return;
		Ref<VulkanStorageBuffer> instance = this;
		Renderer::Submit([instance, data, size, offset]() mutable {
			instance->RT_Update(data.Data, size, offset);
			instance->m_Buffers.PushBack(data);
		});
	}
	void VulkanStorageBuffer::Resize(uint32_t size)
	{
		Ref<VulkanStorageBuffer> instance = this;
		Renderer::Submit([instance, size]() mutable {
			instance->m_Size = size;
			instance->RT_invalidate();
		});
	}

	void VulkanStorageBuffer::SetBufferInfo(uint32_t size, uint32_t offset)
	{
		Ref<VulkanStorageBuffer> instance = this;
		Renderer::Submit([instance, size, offset]() mutable {

			instance->RT_SetBufferInfo(size, offset);

		});
	}
	void VulkanStorageBuffer::RT_SetBufferInfo(uint32_t size, uint32_t offset)
	{
		m_DescriptorInfo.offset = offset;
		m_DescriptorInfo.range = size;
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

		VkBufferUsageFlags flags = VK_BUFFER_USAGE_STORAGE_BUFFER_BIT;
		if (m_IsIndirect || m_Size >= sc_SizeLimit)
		{
			flags |= VK_BUFFER_USAGE_TRANSFER_DST_BIT | VK_BUFFER_USAGE_INDIRECT_BUFFER_BIT;
		}

		VkBufferCreateInfo bufferInfo = {};
		bufferInfo.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
		bufferInfo.usage = flags;
		bufferInfo.size = m_Size;

		VulkanAllocator allocator("StorageBuffer");
		VmaMemoryUsage memoryUsage = m_Size >= sc_SizeLimit ? VMA_MEMORY_USAGE_GPU_ONLY : VMA_MEMORY_USAGE_CPU_TO_GPU;
		m_MemoryAllocation = allocator.AllocateBuffer(bufferInfo, memoryUsage, m_Buffer);

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
		{
			buffer = m_Buffers.PopBack();
			buffer.TryReallocate(m_Size);
		}
		return buffer;
	}

}