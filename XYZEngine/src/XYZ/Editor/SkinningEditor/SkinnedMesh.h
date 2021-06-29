#pragma once
#include "XYZ/Utils/DataStructures/Tree.h"
#include "XYZ/Renderer/Shader.h"
#include "XYZ/Renderer/VertexArray.h"
#include "XYZ/Renderer/Buffer.h"

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
			Submesh(const glm::vec4& color, uint32_t id)
				: Color(color), ID(id)
			{}
			// This vector is filled after triangulation to store the original vertices
			std::vector<BoneVertex> OriginalVertices;
			std::vector<BoneVertex> GeneratedVertices;

			// This vector is filled after initialize vertices local to bone
			std::vector<BoneVertex> VerticesLocalToBones;
			std::vector<Triangle>	Triangles;

			const glm::vec4			Color;
			const uint32_t			ID;

			BoneVertex* FindVertex(const glm::vec2& pos, float radius);
			Triangle* FindTriangle(const glm::vec2& pos, float radius);
		};
	
		class SkinnedMesh
		{
		public:
			SkinnedMesh();
			void Render(Ref<Shader> shader, bool displayWeights);

			void Triangulate();
			void SetContextSize(const glm::vec2& size);
			void BuildPreviewVertices(const Tree& hierarchy, bool preview, bool weight);

			bool EraseVertexAtPosition(const glm::vec2& pos);
			bool EraseTriangleAtPosition(const glm::vec2& pos);
			
			enum Colors
			{
				TriangleColor,
				VertexColor,
				NumColors
			};

			glm::vec4				   m_Colors[NumColors];
			std::vector<Submesh>	   m_Submeshes;
			std::vector<PreviewVertex> m_PreviewVertices;

			static constexpr float PointRadius = 5.0f;
			
			static void GetPositionLocalToBone(BoneVertex& vertex, const Tree& hierarchy);
		private:
			static bool trianglesHaveIndex(const Submesh& subMesh, uint32_t index);
			static void triangulateSubmesh(Submesh& subMesh);
			static void eraseEmptyPoints(Submesh& subMesh);
			static void getPositionFromBones(BoneVertex& vertex, const Tree& hierarchy);
			static void getColorFromBoneWeights(BoneVertex& vertex, const Tree& hierarchy);		
			

			void updateBuffers();
			void rebuildBuffers();

		private:
			Ref<VertexArray> m_VertexArray;
			Ref<VertexBuffer> m_VertexBuffer;

			glm::vec2 m_ContextSize;
		};
	}
}