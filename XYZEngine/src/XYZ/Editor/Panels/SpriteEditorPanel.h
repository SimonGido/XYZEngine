#pragma once
#include "XYZ/Renderer/VertexArray.h"
#include "XYZ/Renderer/Framebuffer.h"
#include "XYZ/Renderer/SubTexture.h"
#include "XYZ/Renderer/RenderTexture.h"

#include "XYZ/Event/Event.h"
#include "XYZ/Event/InputEvent.h"
#include "XYZ/InGui/InGui.h"
#include "XYZ/Core/Timestep.h"
#include "XYZ/Utils/DataStructures/Tree.h"
#include "XYZ/Core/MemoryPool.h"

#include <array>

namespace XYZ {

	class SpriteEditorPanel
	{
	public:
		SpriteEditorPanel(uint32_t panelID);

		void SetContext(Ref<SubTexture> context);

		void OnUpdate(Timestep ts);
		void OnInGuiRender();
		void OnEvent(Event& event);

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
		
		struct Vertex
		{
			float X, Y;
			BoneData Data;
			glm::vec3 Color = glm::vec3(0.1f);
		};

		struct Triangle
		{
			uint32_t First = 0;
			uint32_t Second = 0;
			uint32_t Third = 0;
		};

		struct Bone
		{
			glm::vec2 WorldStart;
			glm::vec2 Start;
			glm::vec2 End;

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
			PreviewPose		= BIT(0),
			CreateBone		= BIT(1),
			EditBone		= BIT(2),
			DeleteBone		= BIT(3),

			CreateVertex	= BIT(4),
			EditVertex		= BIT(5),
			DeleteVertex	= BIT(6),
			DeleteTriangle	= BIT(7),

			WeightBrush		= BIT(8)
		};

	private:
		bool onMouseButtonPress(MouseButtonPressEvent& event);
		bool onMouseButtonRelease(MouseButtonReleaseEvent& event);
		bool onMouseScroll(MouseScrollEvent& event);
		
		void colorizeWeights();
		void updateBoneHierarchy();
		void inGuiRenderBoneHierarchy();
		void submitPreviewsToRenderer();
		void rebuildRenderBuffers();
		void updateVertexBuffer();
		glm::vec2 calculateTexCoord(const glm::vec2& pos);

		void autoWeights();
		void handleWeightBrush(const glm::vec2& pos);
		void initializePose();
		void verticesToBoneLocalSpace();
		void triangulate();
		void eraseEmptyPoints();
		void showTriangle(const Triangle& triangle, const glm::vec4& color);
		void findVerticesInRadius(const glm::vec2& pos, float radius, std::vector<Vertex*>& vertices);
		Triangle findTriangle(const glm::vec2& pos);	
		int32_t findBone(const glm::vec2& pos);

		std::pair<float, float> getMouseViewportSpace() const;
	private:
		const uint32_t m_PanelID;
	
		Ref<SubTexture> m_Context;
		glm::vec2 m_ContextSize = glm::vec2(0.0f);
		
		Tree m_BoneHierarchy;
		MemoryPool m_BonePool;

		std::vector<Bone*> m_Bones;
		std::vector<Vertex> m_Vertices;
		std::vector<glm::vec2> m_VerticesLocalToBones;
		std::vector<uint32_t> m_Indices;

		float m_Scale = 1.0f;
		float m_ScrollOffset = 0.0f;

		Bone* m_CreatedBone = nullptr;
		Bone* m_SelectedBone = nullptr;
		Vertex* m_EditedVertex = nullptr;

		int32_t m_BoneID;
		bool m_FoundBone;

		Triangle m_Triangle;
		bool m_TriangleFound = false;
		bool m_Triangulated = false;

		float m_WeightBrushRadius = 15.0f;
		float m_WeightBrushStrength = 0.01f;

		uint16_t m_Flags = 0;
		bool m_CategoriesOpen[Categories::NumCategories];

		Ref<Framebuffer> m_Framebuffer;
		Ref<RenderTexture> m_RenderTexture;
		Ref<SubTexture> m_RenderSubTexture;
		Ref<VertexArray> m_VertexArray;
		Ref<VertexBuffer> m_VertexBuffer;
		Ref<Material> m_Material;
		Ref<Shader> m_Shader;
		glm::vec2 m_ViewportSize;
		glm::vec2 m_MousePosition;

		static constexpr glm::vec4 sc_VertexColor = glm::vec4(0.0f, 0.7f, 0.8f, 1.0f);
		static constexpr glm::vec4 sc_TriangleColor = glm::vec4(0.8f, 0.8f, 0.8f, 0.5f);
		static constexpr glm::vec4 sc_TriangleHighlightColor = glm::vec4(0.9f, 0.9f, 0.9f, 1.0f);
		static constexpr float sc_PointRadius = 5.0f;
	};
}