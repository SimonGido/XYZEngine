#pragma once
#include "XYZ/Renderer/VertexArray.h"
#include "XYZ/Renderer/Framebuffer.h"
#include "XYZ/Renderer/SubTexture.h"
#include "XYZ/Renderer/RenderTexture.h"
#include "XYZ/Renderer/SkeletalMesh.h"

#include "XYZ/Event/Event.h"
#include "XYZ/Event/InputEvent.h"
#include "XYZ/IG/IG.h"
#include "XYZ/Core/Timestep.h"
#include "XYZ/Utils/DataStructures/Tree.h"
#include "XYZ/Utils/DataStructures/MemoryPool.h"



namespace XYZ {
#define MAX_BONES 60
	class SkinningEditorPanel
	{
	public:
		SkinningEditorPanel();
		void SetContext(Ref<SubTexture> context);

		void OnUpdate(Timestep ts);
		void OnEvent(Event& event);

		void Save();
	private:
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

		struct BoneVertex
		{
			float X, Y;
			BoneData Data;
			glm::vec3 Color = glm::vec3(0.0f);
		};

		struct PreviewVertex
		{
			glm::vec3 Color;
			glm::vec3 Position;
			glm::vec2 TexCoord;
			VertexBoneData BoneData;
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
			std::vector<BoneVertex> Vertices;
			std::vector<Triangle> Triangles;
		};

		struct PreviewBone
		{
			glm::vec2 WorldStart = glm::vec2(0.0f);
			glm::vec2 Start = glm::vec2(0.0f);
			glm::vec2 End = glm::vec2(0.0f);
			int32_t ID;

			glm::mat4 PreviewTransform;
			glm::mat4 PreviewFinalTransform;

			std::string Name;
			glm::vec3   Color;
			bool Open = false;
		};

		enum Categories
		{
			Bones,
			Geometry,
			Weights,
			NumCategories
		};

		enum Flags
		{
			PreviewPose = BIT(0),
			CreateBone = BIT(1),
			EditBone = BIT(2),
			DeleteBone = BIT(3),

			CreateSubMesh = BIT(4),
			CreateVertex = BIT(5),
			EditVertex = BIT(6),
			DeleteVertex = BIT(7),
			DeleteTriangle = BIT(8),

			WeightBrush = BIT(9)
		};

		enum Color
		{
			VertexColor,
			TriangleColor,
			TriangleHighlightColor,
			BoneHighlightColor,
			NumColors
		};
	private:
		void setupUI();
		
	private:
		bool onMouseButtonPress(MouseButtonPressEvent& event);
		bool onMouseButtonRelease(MouseButtonReleaseEvent& event);
		bool onMouseScroll(MouseScrollEvent& event);
		bool onKeyPress(KeyPressedEvent& event);
		void updateCamera(Timestep ts);

		void clear();
		void triangulate(Submesh& subMesh);
		void initializePose();
		void updateBoneHierarchy();
		void updateVertexBuffer();
		void rebuildRenderBuffers();
		void eraseEmptyPoints();
		void eraseVertexAtPosition(const glm::vec2& pos);
		void eraseTriangleAtPosition(const glm::vec2& pos);
		void eraseBone(PreviewBone* bone);
		void setEditBone(PreviewBone* bone);
		void decomposeBone(PreviewBone* bone, glm::vec2& start, glm::vec2& end, float& rot, glm::vec2& normal, bool finalTransform = true);

		void previewBoneCreate();
		void handleBoneCreate();
		void handleBoneEdit();
		void handleVertexEdit();
		void handleWeightsBrush();

		bool trianglesHaveIndex(const Submesh& subMesh, uint32_t index) const;
		glm::vec2 calculateTexCoord(const glm::vec2& pos) const;
		glm::vec2 getPositionLocalToBone(const BoneVertex& vertex);
		glm::vec2 getPositionFromBones(const BoneVertex& vertex);
		glm::vec3 getColorFromBoneWeights(const BoneVertex& vertex);
		glm::vec2 getMouseWindowSpace() const;
		std::pair<float, float> getMouseViewportSpace() const;

		void renderAll();
		void renderMesh(const glm::mat4& viewProjection);
		void renderPreviews(const glm::mat4& viewProjection);
		void renderBoneRelation(PreviewBone* parent, PreviewBone* child);
		void renderTriangle(uint32_t subMeshIndex, uint32_t vertexOffset, const Triangle& triangle, const glm::vec4& color);
		void renderBone(float radius, const glm::vec2& start, const glm::vec2& end, const glm::vec2& normal, const glm::vec4& color);

		std::pair<uint32_t, Triangle*> findTriangle(const glm::vec2& pos);
		std::pair<uint32_t, BoneVertex*> findVertex(const glm::vec2& pos);
		PreviewBone* findBone(const glm::vec2& pos);
		void findVerticesInRadius(const glm::vec2& pos, float radius, std::vector<BoneVertex*>& vertices);
	
private:
		const glm::vec4 m_Colors[Color::NumColors];
		const float m_PointRadius;

		size_t m_HandleCount;
		size_t m_PoolHandle;
		IGImageWindow* m_Window;
		IGTree* m_Tree;

		Ref<SubTexture> m_Context;
		glm::vec2 m_ContextSize = glm::vec2(0.0f);

		Ref<Framebuffer> m_Framebuffer;
		Ref<RenderTexture> m_RenderTexture;
		Ref<SubTexture> m_RenderSubTexture;
		Ref<VertexArray> m_VertexArray;
		Ref<VertexBuffer> m_VertexBuffer;
		Ref<Shader> m_Shader;

		struct Camera
		{
			void UpdateViewProjection();

			glm::mat4 ViewProjectionMatrix = glm::mat4(1.0f);
			glm::mat4 ProjectionMatrix = glm::mat4(1.0f);
			glm::vec2 Position = glm::vec2(0.0f);
			float Speed = 150.0f;
		};

		Ref<SkeletalMesh> m_SkeletalMesh;

		Camera m_Camera;
		glm::vec2 m_ViewportSize;
		glm::vec2 m_MousePosition;

		Tree m_BoneHierarchy;
		MemoryPool m_BonePool;

		std::vector<PreviewBone*> m_Bones;
		std::vector<Submesh> m_SubMeshes;
		std::vector<PreviewVertex> m_PreviewVertices;

		float m_Scale = 1.0f;
		float m_ScrollOffset = 0.0f;

		uint32_t m_FoundSubmeshIndex = 0;
		PreviewBone* m_FoundBone = nullptr;
		BoneVertex* m_FoundVertex = nullptr;
		Triangle* m_FoundTriangle = nullptr;

		uint32_t m_SelectedSubmeshIndex = 0;
		PreviewBone* m_SelectedBone = nullptr;
		BoneVertex* m_SelectedVertex = nullptr;
		Triangle* m_SelectedTriangle = nullptr;

		struct NewBoneData
		{
			PreviewBone* Parent = nullptr;
			glm::vec2 Start = glm::vec2(0.0f);
			bool Creating = false;
		};
		NewBoneData m_NewBoneData;
		
		struct EditBoneData
		{
			PreviewBone* Bone = nullptr;
			bool Rotate = false;
		};
		EditBoneData m_EditBoneData;


		float m_WeightBrushRadius = 15.0f;
		float m_WeightBrushStrength = 0.01f;

		uint16_t m_Flags = 0;
		bool m_Triangulated = false;
		bool m_CategoriesOpen[Categories::NumCategories];
		uint32_t m_ColorIDs[MAX_BONES];
	};
}