#include "stdafx.h"
#include "PreviewRenderer.h"

#include "XYZ/Renderer/Renderer2D.h"

namespace XYZ {
	namespace Editor {
        void PreviewRenderer::RenderSkinnedMesh(const SkinnedMesh& mesh, bool preview)
        {         
            if (preview)
            {
                uint32_t offset = 0;
                for (auto& subMesh : mesh.Submeshes)
                {
                    for (auto& triangle : subMesh.Triangles)
                    {
                        const PreviewVertex& first = mesh.PreviewVertices[(size_t)triangle.First + offset];
                        const PreviewVertex& second = mesh.PreviewVertices[(size_t)triangle.Second + offset];
                        const PreviewVertex& third = mesh.PreviewVertices[(size_t)triangle.Third + offset];
                        RenderTriangle(first.Position, second.Position, third.Position, mesh.Colors[SkinnedMesh::TriangleColor]);
                    }
                    offset += subMesh.GeneratedVertices.size();
                }
                for (auto& vertex : mesh.PreviewVertices)
                    Renderer2D::SubmitCircle(glm::vec3(vertex.Position.x, vertex.Position.y, 0.0f), SkinnedMesh::PointRadius, 20, mesh.Colors[SkinnedMesh::VertexColor]);
            }
            else
            {
                uint32_t counter = 0;
                for (auto& subMesh : mesh.Submeshes)
                {
                    for (auto& triangle : subMesh.Triangles)
                    {
                        const BoneVertex& first = mesh.Submeshes[counter].GeneratedVertices[triangle.First];
                        const BoneVertex& second = mesh.Submeshes[counter].GeneratedVertices[triangle.Second];
                        const BoneVertex& third = mesh.Submeshes[counter].GeneratedVertices[triangle.Third];
                        RenderTriangle(first.Position, second.Position, third.Position, mesh.Colors[SkinnedMesh::TriangleColor]);
                    }
                    counter++;               
                }
                for (auto& subMesh : mesh.Submeshes)
                {
                    for (auto& vertex : subMesh.GeneratedVertices)
                        Renderer2D::SubmitCircle(glm::vec3(vertex.Position.x, vertex.Position.y, 0.0f), SkinnedMesh::PointRadius, 20, subMesh.Color);
                }
            }
        }

        void PreviewRenderer::RenderHierarchy(const Tree& hierarchy, bool transform)
        {
            if (transform)
            {
                hierarchy.Traverse([&](void* parent, void* child) -> bool {

                    PreviewBone* childBone = static_cast<PreviewBone*>(child);
                    float rot;
                    glm::vec2 start, end, normal;
                    childBone->Decompose(start, end, rot, normal);
                    if (parent)
                    {
                        PreviewBone* parentBone = static_cast<PreviewBone*>(parent);
                        float parentRot;
                        glm::vec2 parentStart, parentEnd, parentNormal;
                        parentBone->Decompose(parentStart, parentEnd, parentRot, parentNormal);
                        
                        glm::vec2 dir = glm::normalize(start - parentStart);
                        glm::vec2 relNormal = { -dir.y, dir.x };
                        RenderBone(parentStart, start, relNormal, glm::vec4(childBone->Color, 0.2f), PreviewBone::PointRadius);
                    }              
                    RenderBone(start, end, normal, glm::vec4(childBone->Color, 1.0f), PreviewBone::PointRadius);
                    return false;
                });     
            }
            else
            {
                hierarchy.Traverse([&](void* parent, void* child) -> bool {

                    PreviewBone* childBone = static_cast<PreviewBone*>(child);
                    glm::vec2 end = childBone->LocalPosition + (childBone->Direction * childBone->Length);
                    if (parent)
                    {
                        PreviewBone* parentBone = static_cast<PreviewBone*>(parent);
                        end += parentBone->WorldPosition;
   
                        glm::vec2 dir = glm::normalize(end - parentBone->WorldPosition);
                        glm::vec2 relNormal = { -dir.y, dir.x };
                        RenderBone(parentBone->WorldPosition, childBone->WorldPosition, relNormal, glm::vec4(childBone->Color, 0.2f), PreviewBone::PointRadius);
                    }      
                    glm::vec2 dir = glm::normalize(end - childBone->WorldPosition);
                    glm::vec2 normal = { -dir.y, dir.x };
                    RenderBone(childBone->WorldPosition, end, normal, glm::vec4(childBone->Color, 1.0f), PreviewBone::PointRadius);
                    return false;
                }); 
            }
        }

        void PreviewRenderer::RenderBone(const glm::vec2& start, const glm::vec2& end, const glm::vec2& normal, const glm::vec4& color, float radius)
        {
            Renderer2D::SubmitCircle(glm::vec3(start, 0.0f), radius, 20, color);
            Renderer2D::SubmitCircle(glm::vec3(end, 0.0f), radius, 20, color);
            Renderer2D::SubmitLine(glm::vec3(start + (normal * radius),0.0f), glm::vec3(end, 0.0f), color);
            Renderer2D::SubmitLine(glm::vec3(start - (normal * radius),0.0f), glm::vec3(end, 0.0f), color);
        }

        void PreviewRenderer::RenderBone(const PreviewBone& child, const glm::vec4& color, float radius, bool preview)
        {
            if (preview)
            {
                float rot;
                glm::vec2 start, end, normal;
                child.Decompose(start, end, rot, normal);                       
                RenderBone(start, end, normal, color, radius);
            }
            else
            {
                glm::vec2 end = child.WorldPosition + (child.Direction * child.Length);
                glm::vec2 dir = glm::normalize(end - child.WorldPosition);
                glm::vec2 normal = { -dir.y, dir.x };
                RenderBone(child.WorldPosition, end, normal, color, radius);
            }
        }

        void PreviewRenderer::RenderTriangle(const glm::vec2& firstPosition, const glm::vec2& secondPosition, const glm::vec2& thirdPosition, const glm::vec4& color)
        {
            Renderer2D::SubmitLine(glm::vec3(firstPosition.x, firstPosition.y, 0.0f), glm::vec3(secondPosition.x, secondPosition.y, 0.0f), color);
            Renderer2D::SubmitLine(glm::vec3(secondPosition.x, secondPosition.y, 0.0f), glm::vec3(thirdPosition.x, thirdPosition.y, 0.0f), color);
            Renderer2D::SubmitLine(glm::vec3(thirdPosition.x, thirdPosition.y, 0.0f), glm::vec3(firstPosition.x, firstPosition.y, 0.0f), color);
        }

        void PreviewRenderer::RenderTriangle(const Submesh& mesh, const Triangle& triangle, const glm::vec4& color)
        {
            const BoneVertex& first = mesh.GeneratedVertices[triangle.First];
            const BoneVertex& second = mesh.GeneratedVertices[triangle.Second];
            const BoneVertex& third = mesh.GeneratedVertices[triangle.Third];
            RenderTriangle(first.Position, second.Position, third.Position, color);
        }

	}
}



