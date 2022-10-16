#pragma once
#include "XYZ/Renderer/Buffer.h"
#include "XYZ/Utils/DataStructures/ThreadQueue.h"
#include "XYZ/Utils/DataStructures/ByteBuffer.h"

#include "VulkanAllocator.h"

namespace XYZ {
	class VulkanStorageBuffer : public StorageBuffer
	{
	public:
		VulkanStorageBuffer(uint32_t size, uint32_t binding, bool indirect);
		VulkanStorageBuffer(const void* data, uint32_t size, uint32_t binding, bool indirect);
		virtual ~VulkanStorageBuffer() override;

		virtual void Update(const void* data, uint32_t size, uint32_t offset = 0) override;
		virtual void RT_Update(const void* data, uint32_t size, uint32_t offset = 0) override;
		virtual void Update(ByteBuffer data, uint32_t size, uint32_t offset = 0) override;
		virtual void Resize(uint32_t size) override;
		
		virtual bool		IsIndirect() const override { return m_IsIndirect; }
		virtual uint32_t	GetBinding() const override { return m_Binding; }
		virtual ByteBuffer	GetBuffer() override;


		VkBuffer GetVulkanBuffer() const { return m_Buffer; }
		const VkDescriptorBufferInfo& GetDescriptorBufferInfo() const { return m_DescriptorInfo; }
	private:
		void release();
		void RT_invalidate();

	private:
		VmaAllocation m_MemoryAllocation = nullptr;
		VkBuffer	  m_Buffer{};
		VkDescriptorBufferInfo m_DescriptorInfo{};

		uint32_t		  m_Size;
		uint32_t		  m_Binding;
		BufferLayout	  m_Layout;
		ThreadQueue<ByteBuffer> m_Buffers;

		bool m_IsIndirect;
	};
}