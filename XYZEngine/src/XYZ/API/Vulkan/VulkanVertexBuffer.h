#pragma once
#include "XYZ/Renderer/Buffer.h"
#include "XYZ/Utils/DataStructures/ByteBuffer.h"
#include "VulkanAllocator.h"

namespace  XYZ
{
    class VulkanVertexBuffer : public VertexBuffer
    {
    public:
        VulkanVertexBuffer(const void* vertices, uint32_t size);
        VulkanVertexBuffer(uint32_t size);
        virtual ~VulkanVertexBuffer() override;

        virtual void Update(const void* vertices, uint32_t size, uint32_t offset = 0) override;
        virtual void RT_Update(const void* vertices, uint32_t size, uint32_t offset = 0) override;
        
        virtual void SetUseSize(uint32_t size) override;
        virtual uint32_t GetSize() const override { return m_Size; }
        virtual uint32_t GetUseSize() const override { return m_UseSize; }

        // Vulkan specific
        VkBuffer  GetVulkanBuffer() const { return m_VulkanBuffer; }

    private:
        ByteBuffer prepareBuffer(uint32_t size);

    private:
        uint32_t          m_Size;
        uint32_t          m_UseSize;

        BufferLayout      m_Layout;
        VkBuffer          m_VulkanBuffer;
        ThreadQueue<ByteBuffer> m_Buffers;
        VmaAllocation     m_MemoryAllocation;
    };
}

