#include "stdafx.h"
#include "SkeletalMesh.h"

#include "Renderer.h"
#include "Renderer2D.h"
#include "XYZ/Core/Input.h"

#include <glm/gtx/transform.hpp>
#include <glm/gtx/matrix_decompose.hpp>
#include <glm/gtx/quaternion.hpp>

namespace XYZ {
    static bool s_Pressed = false;

    static uint32_t s_LastSelected = 0;

    static glm::vec3 Interpolate(const glm::vec3& start, const glm::vec3& end, float delta)
    {
        glm::vec3 diff = end - start;
        return diff * delta;
    }

    static glm::mat4 CalculateTransform(const glm::vec3& translation, const glm::vec3& rotation, const glm::vec3& scale)
    {
        glm::mat4 rot = glm::rotate(glm::mat4(1.0f), rotation.x, { 1, 0, 0 })
            * glm::rotate(glm::mat4(1.0f), rotation.y, { 0, 1, 0 })
            * glm::rotate(glm::mat4(1.0f), rotation.z, { 0, 0, 1 });


        return glm::translate(glm::mat4(1.0f), translation)
            * rot
            * glm::scale(glm::mat4(1.0f), scale);
    }

    SkeletalMesh::SkeletalMesh(
        const std::vector<AnimatedVertex>& vertices,
        const std::vector<uint32_t>& indices,
        const std::vector<Bone>& bones,
        const Tree& hierarchy,
        const Ref<Material>& material
    )
        :
        m_Vertices(vertices),
        m_Indices(indices),
        m_Bones(bones),
        m_BoneHierarchy(hierarchy),
        m_Material(material)
    {
        for (auto& bone : m_Bones)
        {
            m_BoneHierarchy.SetData(bone.ID, &bone);
        }
        RebuildBuffers();
        setupFinalTransform();
    }

    SkeletalMesh::SkeletalMesh(
        std::vector<AnimatedVertex>&& vertices, 
        std::vector<uint32_t>&& indices, 
        std::vector<Bone>&& bones,
        Tree&& hierarchy,
        Ref<Material>&& material
    )
        :
        m_Vertices(std::move(vertices)),
        m_Indices(std::move(indices)),
        m_Bones(std::move(bones)),
        m_BoneHierarchy(std::move(hierarchy)),
        m_Material(std::move(material))
    {
        RebuildBuffers();
        setupFinalTransform();
    }

    void SkeletalMesh::Render()
    {
        Ref<Shader> shader = m_Material->GetShader();
        m_Material->Bind();
        shader->SetMat4("u_Transform", glm::translate(glm::vec3(0.0f)));
        shader->SetFloat4("u_Color", glm::vec4(1.0f));

        uint32_t counter = 0;
        for (auto& bone : m_FinalTransformations)
        {
            char name[12];
            sprintf(name, "u_Bones[%u]", counter++);
            shader->SetMat4(name, bone);
        }

        m_VertexArray->Bind();
        Renderer::DrawIndexed(PrimitiveType::Triangles, m_Indices.size());
    }

    void SkeletalMesh::RebuildBuffers()
    {
        m_VertexArray = VertexArray::Create();

        Ref<VertexBuffer> vbo = VertexBuffer::Create(m_Vertices.data(), m_Vertices.size() * sizeof(AnimatedVertex));
        vbo->SetLayout({
            {0, XYZ::ShaderDataComponent::Float3, "a_Position" },
            {1, XYZ::ShaderDataComponent::Float2, "a_TexCoord" },
            {2, XYZ::ShaderDataComponent::Int4,   "a_BoneIDs"  },
            {3, XYZ::ShaderDataComponent::Float4, "a_Weights"  }
            });

        m_VertexArray->AddVertexBuffer(vbo);

        Ref<IndexBuffer> ibo = IndexBuffer::Create(m_Indices.data(), m_Indices.size());
        m_VertexArray->SetIndexBuffer(ibo);
    }
    void SkeletalMesh::setupFinalTransform()
    {
        m_FinalTransformations.resize(m_BoneHierarchy.GetFlatNodes().Range());
        m_BoneHierarchy.Traverse([&](void* parent, void* child) -> bool {

            Bone* childBone = static_cast<Bone*>(child);
            if (parent)
            {
                Bone* parentBone = static_cast<Bone*>(parent);
                childBone->WorldTransform = parentBone->WorldTransform * childBone->Transform;
            }
            else
            {
                childBone->WorldTransform = childBone->Transform;
            }
            m_FinalTransformations[childBone->ID] = childBone->WorldTransform;
            return false;
     });
    }
}