#include "stdafx.h"
#include "SkeletalMesh.h"

#include "Renderer.h"
#include "Renderer2D.h"
#include "XYZ/Core/Input.h"

#include <glm/gtx/transform.hpp>

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


    Joint::Joint(const glm::vec3& position)
        :
        Position(position),
        FinalPosition(position),
        DefaultPosition(position)
    {
    }

    SkeletalAnimation::SkeletalAnimation(const SkeletalAnimation& other)
        :
        Skeleton(other.Skeleton),
        KeyFrames(other.KeyFrames)
    {
        for (auto& joint : Skeleton.Joints)
        {
            Skeleton.JointHierarchy.SetData(joint.ID, &joint);
        }
        for (size_t i = 0; i < other.KeyFrames.size(); ++i)
        {
            for (size_t j = 0; j < other.KeyFrames[i].AffectedJoints.size(); ++j)
            {
                KeyFrames[i].AffectedJoints[j].Joint = static_cast<Joint*>(Skeleton.JointHierarchy.GetData(other.KeyFrames[i].AffectedJoints[j].Joint->ID));
            }
        }
    }

 
    SkeletalMesh::SkeletalMesh(const SkeletalAnimation& animation)
        :
        m_Animation(animation)
    {
        //glm::vec3 quadVertexPositions[4] = {
        //    { -0.5f, -0.5f, 0.0f },
        //    {  0.5f, -0.5f, 0.0f },
        //    {  0.5f,  0.5f, 0.0f },
        //    { -0.5f,  0.5f, 0.0f }
        //};  
        //
        //glm::vec2 texCoords[4] = {
        //    {0.0f, 0.0f},
        //    {1.0f, 0.0f},
        //    {1.0f, 1.0f},
        //    {0.0f, 1.0f}
        //};
        //
        //for (auto& joint : m_Animation.Skeleton.Joints)
        //{
        //    VertexBoneData data;
        //    data.IDs[0] = joint.ID;
        //    for (size_t i = 0; i < 4; ++i)
        //    {
        //        m_Vertices.push_back({
        //            quadVertexPositions[i],
        //            texCoords[i],
        //            data
        //        });
        //    }
        //}
    }
    void SkeletalMesh::Update(float ts)
    {
        if (m_CurrentFrame < m_Animation.KeyFrames.size())
        {
            KeyFrame& keyFrame = m_Animation.KeyFrames[m_CurrentFrame];

            float delta = m_CurrentTime / keyFrame.Length;
            for (auto& it : keyFrame.AffectedJoints)
            {
                it.Joint->Position = Interpolate(it.StartPosition, it.EndPosition, delta);
            }    
            if (m_CurrentTime >= keyFrame.Length)
            {
                m_CurrentTime = 0.0f;
                m_CurrentFrame++;
            }
            m_CurrentTime += ts;
        }
        else if (m_Repeat)
        {
            m_CurrentTime = 0.0f;
            m_CurrentFrame = 0;
        }

        m_Animation.Skeleton.JointHierarchy.Traverse([](void* parent, void* child) -> bool {
           
            Joint* childJoint = static_cast<Joint*>(child);
            childJoint->FinalPosition = childJoint->Position;
            if (parent)
            {
                Joint* parentJoint = static_cast<Joint*>(parent);
                childJoint->FinalPosition = parentJoint->FinalPosition + childJoint->Position;
            }
            return false;
        });

        m_Animation.Skeleton.JointHierarchy.Traverse([&](void* parent, void* child) -> bool {

            Joint* childJoint = static_cast<Joint*>(child);
            glm::vec4 color = glm::vec4(1.0f, 0.0f, 0.0f, 1.0f);
            if (m_Selected && m_Selected->ID == childJoint->ID)
                color = glm::vec4(0.0f, 0.0f, 1.0f, 1.0f);

            Renderer2D::SubmitCircle(childJoint->FinalPosition, 0.05f, 10, color);

            if (parent)
            {
                Joint* parentJoint = static_cast<Joint*>(parent);
                Renderer2D::SubmitLine(parentJoint->FinalPosition, childJoint->FinalPosition, glm::vec4(1.0f, 0.0f, 1.0f, 1.0f));
            }
            return false;
        });

        if (Input::IsKeyPressed(KeyCode::KEY_SPACE) && !s_Pressed)
        {
            s_Pressed = true;
            uint32_t counter = 0;
            m_Animation.Skeleton.JointHierarchy.Traverse([&](void* parent, void* child) -> bool {

                if (counter == s_LastSelected)
                {
                    m_Selected = static_cast<Joint*>(child);
                    s_LastSelected++;
                    if (s_LastSelected >= m_Animation.Skeleton.Joints.size())
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
            m_Selected->Position.x += (mx - m_OldMousePosition.x) / 100.0f;
            m_Selected->Position.y -= (my - m_OldMousePosition.y) / 100.0f;
        }

        auto [mx, my] = Input::GetMousePosition();
        m_OldMousePosition = glm::vec2( mx, my );
    }

    void SkeletalMesh::Render()
    {
        std::vector<AnimatedVertex> vertices;
        for (auto& vertex : m_Vertices)
        {
            Joint* joint = static_cast<Joint*>(m_Animation.Skeleton.JointHierarchy.GetData(vertex.BoneData.IDs[0]));
            vertices.push_back(vertex);
            vertices.back().Position += joint->FinalPosition - joint->DefaultPosition;
        }

        Renderer2D::SubmitQuads(vertices.data(), vertices.size() / 4, 0, glm::vec4(1.0f, 0.0f, 0.0f, 1.0f));
    }

}