#include  "stdafx.h"
#include "VulkanVertexBuffer.h"

#include "VulkanContext.h"

namespace XYZ {
	VulkanVertexBuffer::VulkanVertexBuffer(const void* vertices, uint32_t size, BufferUsage usage)
		:
		m_Size(size),
		m_Usage(usage),
		m_VulkanBuffer(VK_NULL_HANDLE),
		m_MemoryAllocation(VK_NULL_HANDLE)
	{
		ByteBuffer buffer;
		buffer.Allocate(size);
		if (vertices)
			buffer.Write(vertices, size, 0);

		Ref<VulkanVertexBuffer> instance = this;
		Renderer::Submit([instance, buffer]() mutable {

			auto device = VulkanContext::GetCurrentDevice();
			VulkanAllocator allocator("VertexBuffer");
			VkBufferCreateInfo bufferCreateInfo{};
			bufferCreateInfo.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
			bufferCreateInfo.size = instance->m_Size;
			bufferCreateInfo.usage = VK_BUFFER_USAGE_TRANSFER_SRC_BIT;
			bufferCreateInfo.sharingMode = VK_SHARING_MODE_EXCLUSIVE;
			VkBuffer stagingBuffer;
			VmaAllocation stagingBufferAllocation = allocator.AllocateBuffer(bufferCreateInfo, VMA_MEMORY_USAGE_CPU_TO_GPU, stagingBuffer);

			// Copy data to staging buffer
			uint8_t* destData = allocator.MapMemory<uint8_t>(stagingBufferAllocation);
			memcpy(destData, buffer, buffer.m_Size);
			allocator.UnmapMemory(stagingBufferAllocation);

			VkBufferCreateInfo vertexBufferCreateInfo = {};
			vertexBufferCreateInfo.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
			vertexBufferCreateInfo.size = instance->m_Size;
			vertexBufferCreateInfo.usage = VK_BUFFER_USAGE_TRANSFER_DST_BIT | VK_BUFFER_USAGE_VERTEX_BUFFER_BIT;
			instance->m_MemoryAllocation = allocator.AllocateBuffer(vertexBufferCreateInfo, VMA_MEMORY_USAGE_GPU_ONLY, instance->m_VulkanBuffer);

			VkCommandBuffer copyCmd = device->GetCommandBuffer(true);

			VkBufferCopy copyRegion = {};
			copyRegion.size = buffer.m_Size;
			vkCmdCopyBuffer(copyCmd, stagingBuffer, instance->m_VulkanBuffer, 1, &copyRegion);

			device->FlushCommandBuffer(copyCmd);
			allocator.DestroyBuffer(stagingBuffer, stagingBufferAllocation);
			instance->m_Buffers.EmplaceBack(std::move(buffer));
		});
	}
	VulkanVertexBuffer::VulkanVertexBuffer(uint32_t size)
		:
		m_Size(size),
		m_Usage(BufferUsage::Dynamic),
		m_VulkanBuffer(VK_NULL_HANDLE),
		m_MemoryAllocation(VK_NULL_HANDLE)
	{
		ByteBuffer buffer;
		buffer.Allocate(size);
		Ref<VulkanVertexBuffer> instance = this;
		Renderer::Submit([instance, buffer]() mutable
		{
			auto device = VulkanContext::GetCurrentDevice();
			VulkanAllocator allocator("VertexBuffer");

			VkBufferCreateInfo vertexBufferCreateInfo = {};
			vertexBufferCreateInfo.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
			vertexBufferCreateInfo.size = instance->m_Size;
			vertexBufferCreateInfo.usage = VK_BUFFER_USAGE_VERTEX_BUFFER_BIT;

			instance->m_MemoryAllocation = allocator.AllocateBuffer(vertexBufferCreateInfo, VMA_MEMORY_USAGE_CPU_TO_GPU, instance->m_VulkanBuffer);
			instance->m_Buffers.EmplaceBack(std::move(buffer));
		});
	}
	VulkanVertexBuffer::~VulkanVertexBuffer()
	{
		while (!m_Buffers.Empty())
		{
			ByteBuffer buffer = m_Buffers.PopBack();
			delete[]buffer;
		}
		VkBuffer buffer = m_VulkanBuffer;
		VmaAllocation allocation = m_MemoryAllocation;
		Renderer::SubmitResourceFree([buffer, allocation]()
		{
			VulkanAllocator allocator("VertexBuffer");
			allocator.DestroyBuffer(buffer, allocation);
		});
	}
	void VulkanVertexBuffer::Update(const void* vertices, uint32_t size, uint32_t offset)
	{
		ByteBuffer buffer;
		if (m_Buffers.Empty())
			buffer.Allocate(m_Size);
		else
			buffer = m_Buffers.PopBack();

		buffer.Write(vertices, size, offset);
		Ref<VulkanVertexBuffer> instance = this;
		Renderer::Submit([instance, size, offset, buffer]() mutable
		{
			VulkanAllocator allocator("VulkanVertexBuffer");
			uint8_t* pData = allocator.MapMemory<uint8_t>(instance->m_MemoryAllocation);
			memcpy(pData, (uint8_t*)buffer + offset, size);
			allocator.UnmapMemory(instance->m_MemoryAllocation);
		});
	}
	void VulkanVertexBuffer::Resize(const void* vertices, uint32_t size)
	{
	}
	void VulkanVertexBuffer::SetLayout(const BufferLayout& layout)
	{
		m_Layout = layout;
	}
	const BufferLayout& VulkanVertexBuffer::GetLayout() const
	{
		return m_Layout;
	}
}