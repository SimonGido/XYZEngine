#include "stdafx.h"
#include "SkeletalMesh.h"

#include "Renderer.h"
#include "Renderer2D.h"
#include "XYZ/Core/Input.h"

#include <glm/gtx/transform.hpp>
#include <glm/gtx/matrix_decompose.hpp>

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


    Skeleton::Skeleton(const Skeleton& other)
        :
        BoneHierarchy(other.BoneHierarchy),
        Bones(other.Bones)
    {
        for (auto& bone : Bones)
        {
            BoneHierarchy.SetData(bone.ID, &bone);
        }
    }

 
    SkeletalMesh::SkeletalMesh(const Skeleton& skeleton)
        :
        m_Skeleton(skeleton)
    {
    
    }
    void SkeletalMesh::Update(float ts)
    {  
        m_Skeleton.BoneHierarchy.Traverse([](void* parent, void* child) -> bool {
           
            Bone* childBone = static_cast<Bone*>(child);
            if (parent)
            {
                Bone* parentBone = static_cast<Bone*>(parent);
                childBone->FinalTransform = parentBone->FinalTransform * childBone->Transform;
            }
            else
            {
                childBone->FinalTransform = childBone->Transform;
            }
            return false;
        });

        m_Skeleton.BoneHierarchy.Traverse([&](void* parent, void* child) -> bool {

            Bone* childBone = static_cast<Bone*>(child);
            glm::vec4 color = glm::vec4(0.0f, 1.0f, 0.0f, 1.0f);
            if (m_Selected && m_Selected->ID == childBone->ID)
                color = glm::vec4(1.0f, 0.0f, 0.0f, 1.0f);

            glm::vec3 scale;
            glm::quat rotation;
            glm::vec3 translation;
            glm::vec3 skew;
            glm::vec4 perspective;
            glm::decompose(childBone->FinalTransform, scale, rotation, translation, skew, perspective);  
            Renderer2D::SubmitCircle(translation, 0.05f, 10, color);
            
      
            //Renderer2D::SubmitLine(translation, test * childBone->Length, glm::vec4(1.0f, 0.0f, 1.0f, 1.0f));

     
            return false;
        });

      
        if (Input::IsKeyPressed(KeyCode::KEY_SPACE) && !s_Pressed)
        {
            s_Pressed = true;
            uint32_t counter = 0;
            m_Skeleton.BoneHierarchy.Traverse([&](void* parent, void* child) -> bool {
        
                if (counter == s_LastSelected)
                {
                    m_Selected = static_cast<Bone*>(child);
                    std::cout << s_LastSelected << std::endl;
                    s_LastSelected++;
                    if (s_LastSelected >= m_Skeleton.Bones.size())
                        s_LastSelected = 0;
                    return true;
                }
                counter++;
                return false;
            });
        }
        if (!Input::IsKeyPressed(KeyCode::KEY_SPACE))
        {
            s_Pressed = false;
        }
        if (Input::IsMouseButtonPressed(MouseCode::MOUSE_BUTTON_LEFT) && m_Selected)
        {
            auto [mx, my] = Input::GetMousePosition();
            m_Selected->Transform = m_Selected->Transform * glm::rotate(0.001f, glm::vec3(0.0f, 0.0f, 1.0f));
        }
        if (Input::IsMouseButtonPressed(MouseCode::MOUSE_BUTTON_RIGHT) && m_Selected)
        {
            auto [mx, my] = Input::GetMousePosition();

            glm::vec3 translation = glm::vec3((mx - m_OldMousePosition.x) / 100.0f, -(my - m_OldMousePosition.y) / 100.0f, 0.0f);
            m_Selected->Transform = glm::translate(m_Selected->Transform, translation);
        }

        auto [mx, my] = Input::GetMousePosition();
        m_OldMousePosition = glm::vec2( mx, my );
    }

    void SkeletalMesh::Render()
    {
        std::vector<Vertex> vertices;
        vertices.resize(m_Vertices.size());
        uint32_t counter = 0;
        for (auto& vertex : m_Vertices)
        {
            Bone* bone = static_cast<Bone*>(m_Skeleton.BoneHierarchy.GetData(vertex.BoneData.IDs[0]));
            vertices[counter].Position = bone->FinalTransform * glm::vec4(vertex.Position, 1.0f);
            vertices[counter].TexCoord = vertex.TexCoord;
            vertices[counter].Color = glm::vec4(1.0f);
            counter++;
        }
        Renderer2D::SubmitQuads(vertices.data(), vertices.size() / 4, 0);
    }
}