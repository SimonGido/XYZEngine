#pragma once

#include "XYZ/Core/Ref/Ref.h"

#include "VulkanAllocator.h"

namespace XYZ {

	class VulkanBuffer
	{
	public:
		VulkanBuffer();	
		~VulkanBuffer();

		void RT_Create(uint32_t size, VkBufferUsageFlags usage);
		void RT_Create(const void* data, uint32_t size, VkBufferUsageFlags usage);
		void RT_Create(void** data, uint32_t size, VkBufferUsageFlags usage);


		uint32_t GetSize() const { return m_Size; }
		VkBuffer GetVulkanBuffer() const { return m_VulkanBuffer; }
	private:
		void destroy();
		void RT_update(ByteBuffer data);

	private:
		uint32_t		m_Size;
		VkBuffer		m_VulkanBuffer;
		VmaAllocation	m_MemoryAllocation;
		VkBufferUsageFlags m_Usage;
	};
}
