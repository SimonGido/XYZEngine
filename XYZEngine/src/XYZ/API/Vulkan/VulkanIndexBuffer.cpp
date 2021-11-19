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
	}

	VulkanIndexBuffer::VulkanIndexBuffer(const void* indices, uint32_t count, IndexType type)
		: 
		m_Count(count),
		m_Size(count * IndexTypeToSize(type)),
		m_IndexType(IndexTypeToVulkan(type)),
		m_MemoryAllocation(VK_NULL_HANDLE)
	{	
		m_LocalData = ByteBuffer::Copy(indices, m_Size);
		Ref<VulkanIndexBuffer> instance = this;
		Renderer::Submit([instance]() mutable {

			auto device = VulkanContext::GetCurrentDevice();
			VulkanAllocator allocator("IndexBuffer");

			VkBufferCreateInfo bufferCreateInfo{};
			bufferCreateInfo.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
			bufferCreateInfo.size = instance->m_Size;
			bufferCreateInfo.usage = VK_BUFFER_USAGE_TRANSFER_SRC_BIT;
			bufferCreateInfo.sharingMode = VK_SHARING_MODE_EXCLUSIVE;
			VkBuffer stagingBuffer;
			const VmaAllocation stagingBufferAllocation = allocator.AllocateBuffer(bufferCreateInfo, VMA_MEMORY_USAGE_CPU_TO_GPU, stagingBuffer);

			// Copy data to staging buffer
			uint8_t* destData = allocator.MapMemory<uint8_t>(stagingBufferAllocation);
			memcpy(destData, instance->m_LocalData, instance->m_LocalData.m_Size);
			allocator.UnmapMemory(stagingBufferAllocation);

			VkBufferCreateInfo indexBufferCreateInfo = {};
			indexBufferCreateInfo.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
			indexBufferCreateInfo.size = instance->m_Size;
			indexBufferCreateInfo.usage = VK_BUFFER_USAGE_TRANSFER_DST_BIT | VK_BUFFER_USAGE_INDEX_BUFFER_BIT;
			instance->m_MemoryAllocation = allocator.AllocateBuffer(indexBufferCreateInfo, VMA_MEMORY_USAGE_GPU_ONLY, instance->m_VulkanBuffer);

			const VkCommandBuffer copyCmd = device->GetCommandBuffer(true);

			VkBufferCopy copyRegion = {};
			copyRegion.size = instance->m_LocalData.m_Size;
			vkCmdCopyBuffer(copyCmd, stagingBuffer, instance->m_VulkanBuffer, 1, &copyRegion);

			device->FlushCommandBuffer(copyCmd);
			allocator.DestroyBuffer(stagingBuffer, stagingBufferAllocation);
		});
	}

	VulkanIndexBuffer::~VulkanIndexBuffer()
	{
		m_LocalData.Destroy();
		VkBuffer buffer = m_VulkanBuffer;
		VmaAllocation allocation = m_MemoryAllocation;
		Renderer::SubmitResource([buffer, allocation]() {

			VulkanAllocator allocator("IndexBuffer");
			allocator.DestroyBuffer(buffer, allocation);
		});
	}

}