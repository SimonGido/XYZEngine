#include "stdafx.h"
#include "Mesh.h"

#include "XYZ/Renderer/Renderer.h"


namespace XYZ {
   
    StaticMesh::StaticMesh(const Ref<MeshSource>& meshSource)
        :
        m_MeshSource(meshSource)
    {
    }

    AnimatedMesh::AnimatedMesh(const Ref<MeshSource>& meshSource)
        :
        m_MeshSource(meshSource)
    {
    }
    ProceduralMesh::ProceduralMesh()
    {
        m_VertexBuffer = VertexBuffer::Create(0);
        m_IndexBuffer = IndexBuffer::Create(nullptr, 0, IndexType::Uint32);
    }
    void ProceduralMesh::CopyVertices(const void* vertices, uint32_t size)
    {
        if (m_VertexBuffer->GetSize() < size)
        {
            m_VertexBuffer = VertexBuffer::Create(vertices, size);
            return;
        }
        m_VertexBuffer->SetUseSize(size);
        m_VertexBuffer->Update(vertices, size);
    }

    void ProceduralMesh::CopyIndices(const void* indices, uint32_t count, IndexType type)
    {
        if (m_IndexBuffer->GetCount() < count || m_IndexBuffer->GetIndexType() != type)
        {
            m_IndexBuffer = IndexBuffer::Create(indices, count, type);
            return;
        }
        m_IndexBuffer->SetUseCount(count);
        m_IndexBuffer->Update(indices, count);
    }

    void ProceduralMesh::MoveVertices(void** vertices, uint32_t size)
    {
        if (m_VertexBuffer->GetSize() < size)
        {
            m_VertexBuffer = VertexBuffer::Create(vertices, size);
            *vertices = nullptr;
            return;
        }
        m_VertexBuffer->SetUseSize(size);
        void* tempVertices = *vertices;
        *vertices = nullptr;

        Renderer::Submit([buffer = m_VertexBuffer, tempVertices, size]() mutable
        {
            buffer->RT_Update(tempVertices, size);
        });
    }
    
    void ProceduralMesh::MoveIndices(void** indices, uint32_t count, IndexType type)
    {
        if (m_IndexBuffer->GetCount() < count || m_IndexBuffer->GetIndexType() != type)
        {
            m_IndexBuffer = IndexBuffer::Create(indices, count, type);
            *indices = nullptr;
            return;
        }
        m_IndexBuffer->SetUseCount(count);
        void* tempIndices = *indices;
        *indices = nullptr;
        Renderer::Submit([buffer = m_IndexBuffer, tempIndices, count]() mutable
        {
            buffer->RT_Update(tempIndices, count);
        });
    }
}