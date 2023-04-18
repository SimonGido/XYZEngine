#pragma once

#include "XYZ/Core/Ref/Ref.h"

#include "VulkanAllocator.h"

namespace XYZ {

	class VulkanBuffer : public RefCount
	{
	public:
		VulkanBuffer(uint32_t size, VkBufferUsageFlags usage);
		VulkanBuffer(const void* data, uint32_t size, VkBufferUsageFlags usage);
		VulkanBuffer(void** data, uint32_t size, VkBufferUsageFlags usage);
		~VulkanBuffer();

		uint32_t GetSize() const { return m_Size; }
		VkBuffer GetVulkanBuffer() const { return m_VulkanBuffer; }
	private:
		void RT_update(ByteBuffer data);

	private:
		uint32_t		m_Size;
		VkBuffer		m_VulkanBuffer;
		VmaAllocation	m_MemoryAllocation;
		VkBufferUsageFlags m_Usage;
	};
}
