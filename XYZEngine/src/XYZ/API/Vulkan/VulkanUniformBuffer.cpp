#include "stdafx.h"
#include "VulkanUniformBuffer.h"

#include "VulkanContext.h"
#include "VulkanRendererAPI.h"

namespace XYZ {
	VulkanUniformBuffer::VulkanUniformBuffer(uint32_t size, uint32_t binding)
		:
		m_MemoryAlloc(VK_NULL_HANDLE),
		m_Buffer(VK_NULL_HANDLE),
		m_ShaderStage(VK_SHADER_STAGE_FLAG_BITS_MAX_ENUM),
		m_Binding(binding)
	{
		m_LocalStorage.Allocate(size);
		Ref<VulkanUniformBuffer> instance = this;
		Renderer::Submit([instance]() mutable
		{
			instance->RT_invalidate();
		});
	}
	VulkanUniformBuffer::~VulkanUniformBuffer()
	{
		release();
	}
	void VulkanUniformBuffer::Update(const void* data, uint32_t size, uint32_t offset)
	{
		if (size == 0)
			return;
		m_LocalStorage.Write(data, size, offset);
		Ref<VulkanUniformBuffer> instance = this;
		Renderer::Submit([instance, size, offset]() mutable
		{
			instance->RT_Update(instance->m_LocalStorage.Data, size, offset);
		});
	}
	void VulkanUniformBuffer::RT_Update(const void* data, uint32_t size, uint32_t offset)
	{
		if (size == 0)
			return;
		VulkanAllocator allocator("VulkanUniformBuffer");
		uint8_t* pData = allocator.MapMemory<uint8_t>(m_MemoryAlloc);
		memcpy(pData + offset, (const uint8_t*)data, size);
		allocator.UnmapMemory(m_MemoryAlloc);
	}
	
	void VulkanUniformBuffer::RT_SetBufferInfo(uint32_t size, uint32_t offset)
	{
		m_DescriptorInfo.offset = offset;
		m_DescriptorInfo.range = size;
	}

	void VulkanUniformBuffer::RT_invalidate()
	{
		release();

		VkDevice device = VulkanContext::GetCurrentDevice()->GetVulkanDevice();

		VkMemoryAllocateInfo allocInfo = {};
		allocInfo.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
		allocInfo.pNext = nullptr;
		allocInfo.allocationSize = 0;
		allocInfo.memoryTypeIndex = 0;

		VkBufferCreateInfo bufferInfo = {};
		bufferInfo.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
		bufferInfo.usage = VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT;
		bufferInfo.size = m_LocalStorage.Size;

		VulkanAllocator allocator("UniformBuffer");
		m_MemoryAlloc = allocator.AllocateBuffer(bufferInfo, VMA_MEMORY_USAGE_CPU_ONLY, m_Buffer);

		m_DescriptorInfo.buffer = m_Buffer;
		m_DescriptorInfo.offset = 0;
		m_DescriptorInfo.range = m_LocalStorage.Size;
	}
	void VulkanUniformBuffer::release()
	{
		if (!m_MemoryAlloc)
			return;

		Renderer::SubmitResource([buffer = m_Buffer, memoryAlloc = m_MemoryAlloc]()
		{
			VulkanAllocator allocator("UniformBuffer");
			allocator.DestroyBuffer(buffer, memoryAlloc);
		});

		m_Buffer = nullptr;
		m_MemoryAlloc = nullptr;
		m_LocalStorage.Destroy();
	}
}