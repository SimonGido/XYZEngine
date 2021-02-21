#include "stdafx.h"
#include "SkeletalMesh.h"

#include "Renderer.h"
#include "Renderer2D.h"

#include <glm/gtx/transform.hpp>

namespace XYZ {

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
        glm::vec3 quadVertexPositions[4] = {
            { -0.5f, -0.5f, 0.0f },
            {  0.5f, -0.5f, 0.0f },
            {  0.5f,  0.5f, 0.0f },
            { -0.5f,  0.5f, 0.0f }
        };  

        glm::vec2 texCoords[4] = {
            {0.0f, 0.0f},
            {1.0f, 0.0f},
            {1.0f, 1.0f},
            {0.0f, 1.0f}
        };

        for (auto& bone : m_Animation.Skeleton.Joints)
        {
            VertexBoneData data;
            data.IDs[0] = bone.ID;
            for (size_t i = 0; i < 4; ++i)
            {
                m_Vertices.push_back({
                    quadVertexPositions[i],
                    texCoords[i],
                    data
                });
            }
            break;
        }
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
                childJoint->FinalPosition += parentJoint->FinalPosition;
            }
            return false;
        });

        m_Animation.Skeleton.JointHierarchy.Traverse([](void* parent, void* child) -> bool {

            Joint* childJoint = static_cast<Joint*>(child);
            Renderer2D::SubmitCircle(childJoint->FinalPosition, 0.05f, 10, glm::vec4(1.0f, 0.0f, 0.0f, 1.0f));

            if (parent)
            {
                Joint* parentJoint = static_cast<Joint*>(parent);
                Renderer2D::SubmitLine(parentJoint->FinalPosition, childJoint->FinalPosition, glm::vec4(1.0f, 0.0f, 1.0f, 1.0f));
            }
            return false;
        });
    }

    void SkeletalMesh::Render()
    {
        Renderer2D::SubmitQuads(m_Vertices.data(), m_Vertices.size() / 4, 0, glm::vec4(1.0f, 0.0f, 0.0f, 1.0f));
    }

}