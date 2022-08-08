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

		virtual uint32_t GetCount() const override { return m_Count; };
		virtual uint32_t GetRendererID() const override { return 0; };

		VkBuffer    GetVulkanBuffer() const { return m_VulkanBuffer; }
		VkIndexType GetVulkanIndexType() const { return m_IndexType; }
	private:
		uint32_t	  m_Count;
		uint32_t	  m_Size;
		ByteBuffer	  m_LocalData;
		VkIndexType   m_IndexType;
		VkBuffer	  m_VulkanBuffer = nullptr;
		VmaAllocation m_MemoryAllocation;
	};
}