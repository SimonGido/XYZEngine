#include "stdafx.h"
#include "SkinnedMesh.h"

#include "XYZ/Utils/Math/Math.h"
#include "XYZ/Renderer/Renderer2D.h"

#include <tpp_interface.hpp>


namespace XYZ {
	namespace Editor {
        namespace Helper {
            static void GetTriangulationPt(const std::vector<tpp::Delaunay::Point>& points, int keyPointIdx, const tpp::Delaunay::Point& sPoint, double& x, double& y)
            {
                if (keyPointIdx >= points.size())
                {
                    x = sPoint[0]; // added Steiner point, it's data copied to sPoint
                    y = sPoint[1];
                }
                else
                {
                    // point from original data
                    x = sPoint[0];
                    y = sPoint[1];
                }
            }

        }

        BoneVertex* Submesh::FindVertex(const glm::vec2& pos, float radius)
        {  
            for (auto& vertex : Vertices)
            {
                if (glm::distance(pos, vertex.Position) < radius)
                    return &vertex;
            }
            return nullptr;
        }
        Triangle* Submesh::FindTriangle(const glm::vec2& pos, float radius)
        {
            for (auto& triangle : Triangles)
            {
                const BoneVertex& first =  Vertices[triangle.First];
                const BoneVertex& second = Vertices[triangle.Second];
                const BoneVertex& third =  Vertices[triangle.Third];
                if (Math::PointInTriangle(pos, first.Position, second.Position, third.Position))
                    return &triangle;
            }
            return nullptr;
        }
        
        SkinnedMesh::SkinnedMesh()
        {
            Colors[TriangleColor] = glm::vec4(0.8f, 0.8f, 0.8f, 1.0f);
            Colors[VertexColor] = glm::vec4(0.7f, 0.4f, 1.0f, 1.0f);
        }

        void SkinnedMesh::Render()
        {
            RenderTriangles();
            RenderVertices();
        }

        void SkinnedMesh::RenderVertices()
        {
            for (auto& subMesh : Submeshes)
            {
                for (auto& vertex : subMesh.Vertices)
                    Renderer2D::SubmitCircle(glm::vec3(vertex.Position.x, vertex.Position.y, 0.0f), PointRadius, 20, Colors[VertexColor]);
            }
        }

        void SkinnedMesh::RenderTriangles()
        {
            uint32_t counter = 0;
            uint32_t offset = 0;
            for (auto& subMesh : Submeshes)
            {
                for (auto& triangle : subMesh.Triangles)
                {
                    BoneVertex& first =  Submeshes[counter].Vertices[triangle.First];
                    BoneVertex& second = Submeshes[counter].Vertices[triangle.Second];
                    BoneVertex& third =  Submeshes[counter].Vertices[triangle.Third];
                    RenderTriangle(first.Position, second.Position, third.Position, Colors[TriangleColor]);
                }
                counter++;
                offset += subMesh.Vertices.size();
            }
        }

        void SkinnedMesh::Triangulate()
		{
            for (Submesh subMesh : Submeshes)
                triangulateSubmesh(subMesh);
		}
		bool SkinnedMesh::EraseVertexAtPosition(const glm::vec2& pos)
		{
            for (auto& subMesh : Submeshes)
            {
                uint32_t counter = 0;
                for (auto& vertex : subMesh.Vertices)
                {
                    if (glm::distance(pos, glm::vec2(vertex.Position.x, vertex.Position.y)) < PointRadius)
                    {
                        subMesh.Vertices.erase(subMesh.Vertices.begin() + counter);
                        return true;
                    }
                    counter++;
                }
            }
            return false;
		}
		bool SkinnedMesh::EraseTriangleAtPosition(const glm::vec2& pos)
		{
            for (auto& subMesh : Submeshes)
            {
                uint32_t counter = 0;
                for (auto& triangle : subMesh.Triangles)
                {
                    BoneVertex& first =  subMesh.Vertices[triangle.First];
                    BoneVertex& second = subMesh.Vertices[triangle.Second];
                    BoneVertex& third =  subMesh.Vertices[triangle.Third];
                    if (Math::PointInTriangle(pos, glm::vec2(first.Position.x, first.Position.y), glm::vec2(second.Position.x, second.Position.y), glm::vec2(third.Position.x, third.Position.y)))
                    {
                        subMesh.Triangles.erase(subMesh.Triangles.begin() + counter);
                        eraseEmptyPoints(subMesh);
                        return true;
                    }
                    counter++;
                }
            }
            return false;
		}
    
        void SkinnedMesh::RenderTriangle(const glm::vec2& firstPosition, const glm::vec2& secondPosition, const glm::vec2& thirdPosition, const glm::vec4& color)
        {
            Renderer2D::SubmitLine(glm::vec3(firstPosition.x, firstPosition.y, 0.0f), glm::vec3(secondPosition.x, secondPosition.y, 0.0f), color);
            Renderer2D::SubmitLine(glm::vec3(secondPosition.x, secondPosition.y, 0.0f), glm::vec3(thirdPosition.x, thirdPosition.y, 0.0f), color);
            Renderer2D::SubmitLine(glm::vec3(thirdPosition.x, thirdPosition.y, 0.0f), glm::vec3(firstPosition.x, firstPosition.y, 0.0f), color);
        }
       
		bool SkinnedMesh::trianglesHaveIndex(const Submesh& subMesh, uint32_t index)
		{
			for (auto& triangle : subMesh.Triangles)
			{
				if (triangle.First == index || triangle.Second == index || triangle.Third == index)
					return true;
			}
			return false;
		}
		void SkinnedMesh::triangulateSubmesh(Submesh& subMesh)
		{
            if (subMesh.Vertices.size() < 3)
                return;
            std::vector<tpp::Delaunay::Point> points;

            for (auto& p : subMesh.Vertices)
            {
                points.push_back({ p.Position.x, p.Position.y });
            }
            tpp::Delaunay generator(points);
            generator.setMinAngle(30.5f);
            generator.setMaxArea(12000.5f);
            generator.Triangulate(true);

            subMesh.OriginalVertices = std::move(subMesh.Vertices);
            subMesh.Triangles.clear();
            subMesh.Vertices.clear();
            for (tpp::Delaunay::fIterator fit = generator.fbegin(); fit != generator.fend(); ++fit)
            {
                tpp::Delaunay::Point sp1;
                tpp::Delaunay::Point sp2;
                tpp::Delaunay::Point sp3;

                int keypointIdx1 = generator.Org(fit, &sp1);
                int keypointIdx2 = generator.Dest(fit, &sp2);
                int keypointIdx3 = generator.Apex(fit, &sp3);

                double x = 0.0f, y = 0.0f;
                if (!trianglesHaveIndex(subMesh, (uint32_t)keypointIdx1))
                {
                    Helper::GetTriangulationPt(points, keypointIdx1, sp1, x, y);
                    if (subMesh.Vertices.size() <= keypointIdx1)
                        subMesh.Vertices.resize((size_t)keypointIdx1 + 1);
                    subMesh.Vertices[keypointIdx1] = { glm::vec2((float)x, (float)y ) };
                }
                if (!trianglesHaveIndex(subMesh, (uint32_t)keypointIdx2))
                {
                    Helper::GetTriangulationPt(points, keypointIdx2, sp2, x, y);
                    if (subMesh.Vertices.size() <= keypointIdx2)
                        subMesh.Vertices.resize((size_t)keypointIdx2 + 1);
                    subMesh.Vertices[keypointIdx2] = { glm::vec2((float)x, (float)y ) };
                }
                if (!trianglesHaveIndex(subMesh, (uint32_t)keypointIdx3))
                {
                    Helper::GetTriangulationPt(points, keypointIdx3, sp3, x, y);
                    if (subMesh.Vertices.size() <= keypointIdx3)
                        subMesh.Vertices.resize((size_t)keypointIdx3 + 1);
                    subMesh.Vertices[keypointIdx3] = { glm::vec2((float)x, (float)y ) };
                }
                subMesh.Triangles.push_back({
                    (uint32_t)keypointIdx1,
                    (uint32_t)keypointIdx2,
                    (uint32_t)keypointIdx3
                });
            }
		}
        void SkinnedMesh::eraseEmptyPoints(Submesh& subMesh)
        {      
            std::vector<uint32_t> erasedPoints;
            for (uint32_t i = 0; i < subMesh.Vertices.size(); ++i)
            {
                if (!trianglesHaveIndex(subMesh, i))
                    erasedPoints.push_back(i);
            }
            for (int32_t i = erasedPoints.size() - 1; i >= 0; --i)
            {
                subMesh.Vertices.erase(subMesh.Vertices.begin() + erasedPoints[i]);
                for (auto& triange : subMesh.Triangles)
                {
                    if (triange.First >= erasedPoints[i])
                        triange.First--;
                    if (triange.Second >= erasedPoints[i])
                        triange.Second--;
                    if (triange.Third >= erasedPoints[i])
                        triange.Third--;
                }
            }
        }
	}
}