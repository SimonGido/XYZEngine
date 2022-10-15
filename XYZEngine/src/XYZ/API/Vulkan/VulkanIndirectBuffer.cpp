#include "stdafx.h"
#include "VulkanIndirectBuffer.h"

#include "VulkanContext.h"

namespace XYZ {
	VulkanIndirectBuffer::VulkanIndirectBuffer(const void* data, uint32_t size, uint32_t binding)
		:
		m_Size(size),
		m_Binding(binding),
		m_VulkanBuffer(VK_NULL_HANDLE),
		m_MemoryAllocation(VK_NULL_HANDLE)
	{
		ByteBuffer buffer;
		buffer.Allocate(size);
		if (data)
			buffer.Write(data, size, 0);

		Ref<VulkanIndirectBuffer> instance = this;
		Renderer::Submit([instance, buffer]() mutable {

			auto device = VulkanContext::GetCurrentDevice();
			VulkanAllocator allocator("IndirectBuffer");
			VkBufferCreateInfo bufferCreateInfo{};
			bufferCreateInfo.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
			bufferCreateInfo.size = instance->m_Size;
			bufferCreateInfo.usage = VK_BUFFER_USAGE_TRANSFER_SRC_BIT;
			bufferCreateInfo.sharingMode = VK_SHARING_MODE_EXCLUSIVE;
			VkBuffer stagingBuffer;
			const VmaAllocation stagingBufferAllocation = allocator.AllocateBuffer(bufferCreateInfo, VMA_MEMORY_USAGE_CPU_TO_GPU, stagingBuffer);

			// Copy data to staging buffer
			uint8_t* destData = allocator.MapMemory<uint8_t>(stagingBufferAllocation);
			memcpy(destData, buffer, buffer.Size);
			allocator.UnmapMemory(stagingBufferAllocation);

			VkBufferCreateInfo indirectBufferCreateInfo = {};
			indirectBufferCreateInfo.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
			indirectBufferCreateInfo.size = instance->m_Size;
			indirectBufferCreateInfo.usage = VK_BUFFER_USAGE_TRANSFER_DST_BIT | VK_BUFFER_USAGE_STORAGE_BUFFER_BIT | VK_BUFFER_USAGE_INDIRECT_BUFFER_BIT;
			instance->m_MemoryAllocation = allocator.AllocateBuffer(indirectBufferCreateInfo, VMA_MEMORY_USAGE_CPU_TO_GPU, instance->m_VulkanBuffer);
			
			const VkCommandBuffer copyCmd = device->GetCommandBuffer(true);

			VkBufferCopy copyRegion = {};
			copyRegion.size = buffer.Size;
			vkCmdCopyBuffer(copyCmd, stagingBuffer, instance->m_VulkanBuffer, 1, &copyRegion);

			device->FlushCommandBuffer(copyCmd);
			allocator.DestroyBuffer(stagingBuffer, stagingBufferAllocation);
			instance->m_Buffers.EmplaceBack(std::move(buffer));

			instance->m_DescriptorInfo.buffer = instance->m_VulkanBuffer;
			instance->m_DescriptorInfo.offset = 0;
			instance->m_DescriptorInfo.range = instance->m_Size;
		});
	}

	VulkanIndirectBuffer::~VulkanIndirectBuffer()
	{
		while (!m_Buffers.Empty())
		{
			ByteBuffer buffer = m_Buffers.PopBack();
			delete[]buffer;
		}
		VkBuffer buffer = m_VulkanBuffer;
		VmaAllocation allocation = m_MemoryAllocation;
		Renderer::SubmitResource([buffer, allocation]()
		{
			VulkanAllocator allocator("VertexBuffer");
			allocator.DestroyBuffer(buffer, allocation);
		});
	}
}

