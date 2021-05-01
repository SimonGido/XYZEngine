#pragma once
#include "XYZ/Utils/DataStructures/Tree.h"

#include <glm/glm.hpp>

namespace XYZ {
	namespace Editor {

		struct BoneData
		{
			static constexpr uint32_t sc_MaxBonesPerVertex = 4;
			BoneData()
			{
				Reset();
			}
			void Reset()
			{
				memset(IDs, -1, sc_MaxBonesPerVertex * sizeof(int32_t));
				memset(Weights, 0, sc_MaxBonesPerVertex * sizeof(float));
			}
			int32_t IDs[sc_MaxBonesPerVertex];
			float   Weights[sc_MaxBonesPerVertex];
		};

		// Used while editing mesh
		struct BoneVertex
		{
			BoneVertex();
			BoneVertex(const glm::vec2& pos);
			BoneVertex(const BoneVertex& other);

			glm::vec2 Position;
			BoneData  Data;
			glm::vec3 Color;
		};

		// Used to actually draw mesh in shader
		struct PreviewVertex
		{
			glm::vec3 Color;
			glm::vec3 Position;
			glm::vec2 TexCoord;
		};

		struct Triangle
		{
			uint32_t First = 0;
			uint32_t Second = 0;
			uint32_t Third = 0;
			bool operator==(const Triangle& other) const
			{
				return First == other.First 
					&& Second == other.Second 
					&& Third == other.Third;
			}
		};

		struct Submesh
		{
			// This vector is filled after triangulation to store the original vertices
			std::vector<BoneVertex> OriginalVertices;
			std::vector<BoneVertex> Vertices;
			std::vector<Triangle>	Triangles;

			BoneVertex* FindVertex(const glm::vec2& pos, float radius);
			Triangle* FindTriangle(const glm::vec2& pos, float radius);
		};
	
		class SkinnedMesh
		{
		public:
			SkinnedMesh();
			void Triangulate();
			bool EraseVertexAtPosition(const glm::vec2& pos);
			bool EraseTriangleAtPosition(const glm::vec2& pos);
			void BuildPreviewVertices(const Tree& hierarchy, const glm::vec2& contextSize, bool preview, bool weight);
			
			std::vector<Submesh> Submeshes;
			std::vector<PreviewVertex> PreviewVertices;

			enum Colors
			{
				TriangleColor,
				VertexColor,
				NumColors
			};

			glm::vec4 Colors[NumColors];
			static constexpr float PointRadius = 5.0f;
		private:
			static bool trianglesHaveIndex(const Submesh& subMesh, uint32_t index);
			static void triangulateSubmesh(Submesh& subMesh);
			static void eraseEmptyPoints(Submesh& subMesh);
			static void getPositionLocalToBone(BoneVertex& vertex, const Tree& hierarchy);
			static void getPositionFromBones(BoneVertex& vertex, const Tree& hierarchy);
			static void getColorFromBoneWeights(BoneVertex& vertex, const Tree& hierarchy);
		
			
		};
	}
}