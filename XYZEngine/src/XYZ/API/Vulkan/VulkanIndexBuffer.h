#pragma once
#include "XYZ/Renderer/Buffer.h"
#include "XYZ/Utils/DataStructures/ByteBuffer.h"
#include "VulkanAllocator.h"

namespace XYZ {
	class VulkanIndexBuffer : public IndexBuffer
	{
	public:
		VulkanIndexBuffer(const void* indices, uint32_t count, IndexType type);
		virtual ~VulkanIndexBuffer() override;

		virtual void Bind() const override {};
		virtual void UnBind() const override {};
		virtual uint32_t GetRendererID() const override { return 0; };


		virtual void Update(const void* indices, uint32_t count, uint32_t offset = 0) override;
		virtual void RT_Update(const void* indices, uint32_t count, uint32_t offset = 0) override;
		virtual void SetUseCount(uint32_t count) override;
		virtual uint32_t GetCount() const override { return m_Count; };
		virtual uint32_t GetUseCount() const override { return m_UseCount; }
		virtual IndexType GetIndexType() const override { return m_IndexType; }

		VkBuffer    GetVulkanBuffer() const { return m_VulkanBuffer; }
		VkIndexType GetVulkanIndexType() const { return m_VulkanIndexType; }

	private:
		ByteBuffer prepareBuffer(uint32_t size);

	private:
		uint32_t	  m_Count;
		uint32_t	  m_UseCount;
		uint32_t	  m_Size;
		uint32_t	  m_IndexSize;
		IndexType	  m_IndexType;
		ThreadQueue<ByteBuffer> m_Buffers;
		VkIndexType   m_VulkanIndexType;
		VkBuffer	  m_VulkanBuffer = nullptr;
		VmaAllocation m_MemoryAllocation;
	};
}