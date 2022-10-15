#pragma once
#include "XYZ/Renderer/Buffer.h"
#include "XYZ/Utils/DataStructures/ByteBuffer.h"
#include "VulkanAllocator.h"


namespace XYZ {
	class VulkanIndirectBuffer : public IndirectBuffer
	{
	public:
		VulkanIndirectBuffer(const void* data, uint32_t size, uint32_t binding);
		virtual ~VulkanIndirectBuffer() override;

		virtual uint32_t GetBinding() const override { return m_Binding; }

		VkBuffer  GetVulkanBuffer() const { return m_VulkanBuffer; }
	
		const VkDescriptorBufferInfo& GetDescriptorBufferInfo() const { return m_DescriptorInfo; }
	private:
		uint32_t				m_Size;
		uint32_t				m_Binding;
		VkBuffer				m_VulkanBuffer;
		VkDescriptorBufferInfo m_DescriptorInfo{};

		ThreadQueue<ByteBuffer> m_Buffers;
		VmaAllocation			m_MemoryAllocation;
	};
}