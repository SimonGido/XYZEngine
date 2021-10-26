#include "stdafx.h"
#include "Mesh.h"


namespace XYZ {
    Mesh::Mesh(const Ref<Material>& material)
        :
        m_Material(material)
    {
    }
    void Mesh::SetMaterial(const Ref<Material>& material)
    {
        m_Material = material;
    }
    void Mesh::SetIndices(uint32_t* indices, uint32_t count)
    {
        m_IndexBuffer = IndexBuffer::Create(indices, count);
        updateVertexArray();
    }

    void Mesh::SetVertexBufferData(uint32_t index, void* vertices, uint32_t size, uint32_t offset)
    {
        m_VertexBuffers[index]->Update(vertices, size, offset);
    }

    void Mesh::AddVertexBuffer(const BufferLayout& layout, void* vertices, uint32_t size, BufferUsage usage)
    {
        Ref<VertexBuffer> buffer = VertexBuffer::Create(vertices, size, usage);
        buffer->SetLayout(layout);
        m_VertexBuffers.push_back(buffer);
        
        updateVertexArray();
    }
    
    void Mesh::ClearVertexBuffers()
    {
        m_VertexArray = VertexArray::Create();
        m_VertexArray->SetIndexBuffer(m_IndexBuffer);
    }
    void Mesh::updateVertexArray()
    {
        m_VertexArray = VertexArray::Create();
        for (const auto& vbo : m_VertexBuffers)
            m_VertexArray->AddVertexBuffer(vbo);

        m_VertexArray->SetIndexBuffer(m_IndexBuffer);
    }
}