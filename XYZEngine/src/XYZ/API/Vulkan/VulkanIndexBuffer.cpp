#include "stdafx.h"
#include "VulkanIndexBuffer.h"
#include "VulkanContext.h"

namespace XYZ {
	static uint32_t IndexTypeToSize(IndexType type)
	{
		switch (type)
		{
		case XYZ::IndexType::Uint8:
			return sizeof(uint8_t);
		case XYZ::IndexType::Uint16:
			return sizeof(uint16_t);
		case XYZ::IndexType::Uint32:
			return sizeof(uint32_t);
		}
		return 0;
	}

	static VkIndexType IndexTypeToVulkan(IndexType type)
	{
		switch (type)
		{
		case XYZ::IndexType::Uint8:
			return VK_INDEX_TYPE_UINT8_EXT;
		case XYZ::IndexType::Uint16:
			return VK_INDEX_TYPE_UINT16;
		case XYZ::IndexType::Uint32:
			return VK_INDEX_TYPE_UINT32;
		}
		XYZ_ASSERT(false, "Invalid index type");
		return VK_INDEX_TYPE_NONE_NV;
	}

	VulkanIndexBuffer::VulkanIndexBuffer(const void* indices, uint32_t count, IndexType type)
		: 
		m_Count(count),
		m_UseCount(count),
		m_Size(count * IndexTypeToSize(type)),
		m_IndexSize(IndexTypeToSize(type)),
		m_IndexType(type),
		m_VulkanIndexType(IndexTypeToVulkan(type)),
		m_MemoryAllocation(VK_NULL_HANDLE)
	{	
		ByteBuffer buffer = prepareBuffer(m_Size);
		if (indices)
			buffer.Write(indices, m_Size, 0);

		Ref<VulkanIndexBuffer> instance = this;
		Renderer::Submit([instance, buffer]() mutable {

			auto device = VulkanContext::GetCurrentDevice();
			VulkanAllocator allocator("VulkanIndexBuffer");

			VkBufferCreateInfo bufferCreateInfo{};
			bufferCreateInfo.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
			bufferCreateInfo.size = instance->m_Size;
			bufferCreateInfo.usage = VK_BUFFER_USAGE_TRANSFER_SRC_BIT;
			bufferCreateInfo.sharingMode = VK_SHARING_MODE_EXCLUSIVE;
			VkBuffer stagingBuffer;
			const VmaAllocation stagingBufferAllocation = allocator.AllocateBuffer(bufferCreateInfo, VMA_MEMORY_USAGE_CPU_TO_GPU, stagingBuffer);

			// Copy data to staging buffer
			uint8_t* destData = allocator.MapMemory<uint8_t>(stagingBufferAllocation);
			memcpy(destData, buffer.Data, instance->m_Size);
			allocator.UnmapMemory(stagingBufferAllocation);

			VkBufferCreateInfo indexBufferCreateInfo = {};
			indexBufferCreateInfo.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO ;
			indexBufferCreateInfo.size = instance->m_Size;
			indexBufferCreateInfo.usage = VK_BUFFER_USAGE_TRANSFER_DST_BIT | VK_BUFFER_USAGE_INDEX_BUFFER_BIT | VK_BUFFER_USAGE_SHADER_DEVICE_ADDRESS_BIT;
			instance->m_MemoryAllocation = allocator.AllocateBuffer(indexBufferCreateInfo, VMA_MEMORY_USAGE_GPU_ONLY, instance->m_VulkanBuffer);

			const VkCommandBuffer copyCmd = device->GetCommandBuffer(true);

			VkBufferCopy copyRegion = {};
			copyRegion.size = instance->m_Size;
			vkCmdCopyBuffer(copyCmd, stagingBuffer, instance->m_VulkanBuffer, 1, &copyRegion);

			device->FlushCommandBuffer(copyCmd);
			allocator.DestroyBuffer(stagingBuffer, stagingBufferAllocation);
			instance->m_Buffers.EmplaceBack(std::move(buffer));
		});
	}

	VulkanIndexBuffer::~VulkanIndexBuffer()
	{
		while (!m_Buffers.Empty())
		{
			ByteBuffer buffer = m_Buffers.PopBack();
			delete[]buffer;
		}
		VkBuffer buffer = m_VulkanBuffer;
		VmaAllocation allocation = m_MemoryAllocation;
		Renderer::SubmitResource([buffer, allocation]() {

			VulkanAllocator allocator("VulkanIndexBuffer");
			allocator.DestroyBuffer(buffer, allocation);
		});
	}


	void VulkanIndexBuffer::Update(const void* indices, uint32_t count, uint32_t offset)
	{
		XYZ_ASSERT(count + offset <= m_Count, "");
		if (count == 0)
			return;

		uint32_t writeSize = count * m_IndexSize;
		uint32_t writeOffset = offset * m_IndexSize;

		ByteBuffer buffer = prepareBuffer(writeSize);
		buffer.Write(indices, writeSize);

		Ref<VulkanIndexBuffer> instance = this;
		Renderer::Submit([instance, writeSize, writeOffset, buffer]() mutable
		{
			VulkanAllocator allocator("VulkanIndexBuffer");
			uint8_t* pData = allocator.MapMemory<uint8_t>(instance->m_MemoryAllocation);
			memcpy(pData + writeOffset, buffer.Data, writeSize);
			allocator.UnmapMemory(instance->m_MemoryAllocation);
			instance->m_Buffers.PushFront(buffer);
		});
	}

	void VulkanIndexBuffer::RT_Update(const void* indices, uint32_t count, uint32_t offset)
	{
		if (count == 0)
			return;

		uint32_t writeSize = count * m_IndexSize;
		uint32_t writeOffset = offset * m_IndexSize;

		VulkanAllocator allocator("VulkanIndexBuffer");
		uint8_t* pData = allocator.MapMemory<uint8_t>(m_MemoryAllocation);
		memcpy(pData + writeOffset, (uint8_t*)indices, writeSize);
		allocator.UnmapMemory(m_MemoryAllocation);
	}

	void VulkanIndexBuffer::SetUseCount(uint32_t count)
	{
		XYZ_ASSERT(m_Count >= count, "");

		Ref<VulkanIndexBuffer> instance = this;
		Renderer::Submit([instance, count]() mutable {
			instance->m_UseCount = count;
		});
	}

	ByteBuffer VulkanIndexBuffer::prepareBuffer(uint32_t size)
	{
		ByteBuffer buffer;
		if (!m_Buffers.Empty())
			buffer = m_Buffers.PopBack();

		if (buffer.Size < size)
			buffer.Allocate(size);

		return buffer;
	}

}