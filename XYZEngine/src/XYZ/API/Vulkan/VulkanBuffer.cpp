#include "stdafx.h"
#include "VulkanBuffer.h"

#include "VulkanContext.h"

namespace XYZ {


	VulkanBuffer::VulkanBuffer(uint32_t size, VkBufferUsageFlags usage)
		:
		m_Size(size),
		m_VulkanBuffer(VK_NULL_HANDLE),
		m_MemoryAllocation(VK_NULL_HANDLE),
		m_Usage(usage)
	{
		Ref<VulkanBuffer> instance = this;
		Renderer::Submit([instance]() mutable {
			instance->RT_update(ByteBuffer());
		});
	}
	VulkanBuffer::VulkanBuffer(const void* data, uint32_t size, VkBufferUsageFlags usage)
		:
		m_Size(size),
		m_VulkanBuffer(VK_NULL_HANDLE),
		m_MemoryAllocation(VK_NULL_HANDLE),
		m_Usage(usage)
	{
		ByteBuffer buffer = ByteBuffer::Copy(data, size);
		Ref<VulkanBuffer> instance = this;
		Renderer::Submit([instance, buffer]() mutable {
			instance->RT_update(buffer);
		});
	}
	VulkanBuffer::VulkanBuffer(void** data, uint32_t size, VkBufferUsageFlags usage)
		:
		m_Size(size),
		m_VulkanBuffer(VK_NULL_HANDLE),
		m_MemoryAllocation(VK_NULL_HANDLE),
		m_Usage(usage)
	{
		ByteBuffer buffer = ByteBuffer(static_cast<uint8_t*>(*data), size);
		*data = nullptr;
		Ref<VulkanBuffer> instance = this;
		Renderer::Submit([instance, buffer]() mutable {
			instance->RT_update(buffer);
			});
	}
	VulkanBuffer::~VulkanBuffer()
	{
		VkBuffer buffer = m_VulkanBuffer;
		VmaAllocation allocation = m_MemoryAllocation;
		Renderer::SubmitResource([buffer, allocation]()
		{
			VulkanAllocator allocator("VulkanBuffer");
			allocator.DestroyBuffer(buffer, allocation);
		});
	}
	void VulkanBuffer::RT_update(ByteBuffer data)
	{
		auto device = VulkanContext::GetCurrentDevice();
		
		VulkanAllocator allocator("VulkanBuffer");
		VkBufferCreateInfo bufferCreateInfo = {};
		bufferCreateInfo.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
		bufferCreateInfo.size = m_Size;
		bufferCreateInfo.usage = m_Usage | VK_BUFFER_USAGE_TRANSFER_DST_BIT;
		m_MemoryAllocation = allocator.AllocateBuffer(bufferCreateInfo, VMA_MEMORY_USAGE_GPU_ONLY, m_VulkanBuffer);

		
		if (data.Data != nullptr)
		{
			VkBuffer stagingBuffer;
			VkBufferCreateInfo stagingBufferCreateInfo{};
			stagingBufferCreateInfo.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
			stagingBufferCreateInfo.size = m_Size;
			stagingBufferCreateInfo.usage = VK_BUFFER_USAGE_TRANSFER_SRC_BIT;
			stagingBufferCreateInfo.sharingMode = VK_SHARING_MODE_EXCLUSIVE;
			const VmaAllocation stagingBufferAllocation = allocator.AllocateBuffer(stagingBufferCreateInfo, VMA_MEMORY_USAGE_CPU_TO_GPU, stagingBuffer);

			// Copy data to staging buffer
			uint8_t* destData = allocator.MapMemory<uint8_t>(stagingBufferAllocation);
			memcpy(destData, data.Data, m_Size);
			allocator.UnmapMemory(stagingBufferAllocation);

			const VkCommandBuffer copyCmd = device->GetCommandBuffer(true);

			VkBufferCopy copyRegion = {};
			copyRegion.size = m_Size;
			vkCmdCopyBuffer(copyCmd, stagingBuffer, m_VulkanBuffer, 1, &copyRegion);

			device->FlushCommandBuffer(copyCmd);
			allocator.DestroyBuffer(stagingBuffer, stagingBufferAllocation);
			data.Destroy();
		}
	}
}
