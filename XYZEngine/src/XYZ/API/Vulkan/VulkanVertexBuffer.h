#pragma once
#include "XYZ/Renderer/Buffer.h"
#include "XYZ/Utils/DataStructures/ByteBuffer.h"
#include "VulkanAllocator.h"
namespace  XYZ
{
    class VulkanVertexBuffer : public VertexBuffer
    {
    public:
        VulkanVertexBuffer(const void* vertices, uint32_t size, BufferUsage usage = BufferUsage::Static);
        VulkanVertexBuffer(uint32_t size);

        virtual void Bind() const override{};
        virtual void UnBind() const override{};
        virtual void Update(const void* vertices, uint32_t size, uint32_t offset = 0) override;
        virtual void Resize(const void* vertices, uint32_t size) override;
        virtual void SetLayout(const BufferLayout& layout) override;
        virtual const BufferLayout& GetLayout() const override;
        virtual uint32_t GetRendererID() const override { return 0;};
    private:
        uint32_t          m_Size;
        BufferUsage       m_Usage;
        BufferLayout      m_Layout;
        Queue<ByteBuffer> m_Buffers;
        VmaAllocation     m_MemoryAllocation;
    };
}

