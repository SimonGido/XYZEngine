#include "stdafx.h"
#include "Mesh.h"


namespace XYZ {
   
    Mesh::Mesh(std::vector<Vertex> vertices, std::vector<uint32_t> indices)
        :
        m_Vertices(std::move(vertices)),
        m_Indices(std::move(indices))
    {
        m_VertexBuffer = VertexBuffer::Create(m_Vertices.data(), static_cast<uint32_t>(m_Vertices.size() * sizeof(Vertex)));
        m_IndexBuffer = IndexBuffer::Create(m_Indices.data(), static_cast<uint32_t>(m_Indices.size()));
    }

    Mesh::~Mesh()
    {
    }
    AnimatedMesh::AnimatedMesh(std::vector<AnimatedVertex> vertices, std::vector<uint32_t> indices)
        :
        m_Vertices(std::move(vertices)),
        m_Indices(std::move(indices))
    {
        m_VertexBuffer = VertexBuffer::Create(m_Vertices.data(), static_cast<uint32_t>(m_Vertices.size() * sizeof(AnimatedVertex)));
        m_IndexBuffer = IndexBuffer::Create(m_Indices.data(), static_cast<uint32_t>(m_Indices.size()));
    }
}