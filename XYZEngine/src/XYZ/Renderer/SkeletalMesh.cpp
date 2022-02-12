#include "stdafx.h"
#include "SkeletalMesh.h"

#include "Renderer.h"
#include "Renderer2D.h"
#include "XYZ/Scene/SceneEntity.h"
#include "XYZ/Scene/Components.h"

#include <glm/gtx/transform.hpp>
#include <glm/gtx/matrix_decompose.hpp>
#include <glm/gtx/quaternion.hpp>

namespace XYZ {
    static bool s_Pressed = false;

    static uint32_t s_LastSelected = 0;

    static glm::vec3 Interpolate(const glm::vec3& start, const glm::vec3& end, float delta)
    {
        const glm::vec3 diff = end - start;
        return diff * delta;
    }

    static glm::mat4 CalculateTransform(const glm::vec3& translation, const glm::vec3& rotation, const glm::vec3& scale)
    {
        const glm::mat4 rot = glm::rotate(glm::mat4(1.0f), rotation.x, { 1, 0, 0 })
            * glm::rotate(glm::mat4(1.0f), rotation.y, { 0, 1, 0 })
            * glm::rotate(glm::mat4(1.0f), rotation.z, { 0, 0, 1 });


        return glm::translate(glm::mat4(1.0f), translation)
            * rot
            * glm::scale(glm::mat4(1.0f), scale);
    }

    SkeletalMesh::SkeletalMesh(
        const std::vector<AnimatedVertex>& vertices,
        const std::vector<uint32_t>& indices,
        const Ref<Material>& material
    )
        :
        m_Vertices(vertices),
        m_Indices(indices),
        m_Material(material)
    {
        RebuildBuffers();
    }

    SkeletalMesh::SkeletalMesh(
        std::vector<AnimatedVertex>&& vertices, 
        std::vector<uint32_t>&& indices, 
        Ref<Material>&& material
    )
        :
        m_Vertices(std::move(vertices)),
        m_Indices(std::move(indices)),
        m_Material(std::move(material))
    {
        RebuildBuffers();
    }

    void SkeletalMesh::Render()
    {
        Ref<Shader> shader = m_Material->GetShader();
   
        shader->SetMat4("u_Transform", glm::translate(glm::vec3(0.0f)));
        shader->SetFloat4("u_Color", glm::vec4(1.0f));

        uint32_t counter = 0;
        for (const SceneEntity& boneEntity : m_Bones)
        {
            const glm::mat4& transform = boneEntity.GetComponent<TransformComponent>().WorldTransform;
            char name[12];
            sprintf(name, "u_Bones[%u]", counter++);
            shader->SetMat4(name, transform);
        }

        m_VertexArray->Bind();
        Renderer::DrawIndexed(PrimitiveType::Triangles, (uint32_t)m_Indices.size());
    }

    void SkeletalMesh::RebuildBuffers()
    {
        m_VertexArray = VertexArray::Create();

        Ref<VertexBuffer> vbo = VertexBuffer::Create(m_Vertices.data(), (uint32_t)m_Vertices.size() * (uint32_t)sizeof(AnimatedVertex));
        vbo->SetLayout({
            {XYZ::ShaderDataType::Float3, "a_Position" },
            {XYZ::ShaderDataType::Float2, "a_TexCoord" },
            {XYZ::ShaderDataType::Int4,   "a_BoneIDs"  },
            {XYZ::ShaderDataType::Float4, "a_Weights"  }
            });

        m_VertexArray->AddVertexBuffer(vbo);

        const Ref<IndexBuffer> ibo = IndexBuffer::Create(m_Indices.data(), (uint32_t)m_Indices.size());
        m_VertexArray->SetIndexBuffer(ibo);
    }
}