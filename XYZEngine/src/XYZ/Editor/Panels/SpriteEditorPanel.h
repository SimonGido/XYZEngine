#pragma once
#include "XYZ/Renderer/VertexArray.h"
#include "XYZ/Renderer/Framebuffer.h"
#include "XYZ/Renderer/SubTexture.h"
#include "XYZ/Renderer/RenderTexture.h"
#include "XYZ/Event/Event.h"
#include "XYZ/Event/InputEvent.h"
#include "XYZ/InGui/InGui.h"
#include "XYZ/Core/Timestep.h"
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
		struct Vertex
		{
			float X, Y;
		};

		struct Triangle
		{
			uint32_t First = 0;
			uint32_t Second = 0;
			uint32_t Third = 0;
		};

		enum Categories
		{
			Bones,
			Geometry,
			Weights,
			NumCategories
		};

		enum GeometryCategoryFlags
		{
			CreateVertex	= BIT(0),
			EditVertex		= BIT(1),
			DeleteVertex	= BIT(2),
			DeleteTriangle	= BIT(3)
		};

	private:
		bool onMouseButtonPress(MouseButtonPressEvent& event);
		bool onMouseButtonRelease(MouseButtonReleaseEvent& event);
		bool onMouseScroll(MouseScrollEvent& event);
		void rebuildRenderBuffers();
		void updateVertexBuffer();
		glm::vec2 calculateTexCoord(const glm::vec2& pos);

		void triangulate();
		void eraseEmptyPoints();
		void showTriangle(const Triangle& triangle, const glm::vec4& color);
		Triangle findTriangle(const glm::vec2& pos);

		std::pair<float, float> getMouseViewportSpace() const;
	private:
		const uint32_t m_PanelID;

		
		Ref<SubTexture> m_Context;
		glm::vec2 m_ContextSize = glm::vec2(0.0f);
		
			
		std::vector<Vertex> m_Vertices;
		std::vector<uint32_t> m_Indices;

		float m_Scale = 1.0f;
		float m_ScrollOffset = 0.0f;

		Vertex* m_EditedVertex = nullptr;

		Triangle m_Triangle;
		bool m_TriangleFound = false;
		bool m_Triangulated = false;


		uint8_t m_GeometryCategoryFlags = 0;

		bool m_CategoriesOpen[Categories::NumCategories];

		static constexpr float sc_PointRadius = 5.0f;


		Ref<Framebuffer> m_Framebuffer;
		Ref<RenderTexture> m_RenderTexture;
		Ref<SubTexture> m_RenderSubTexture;
		Ref<VertexArray> m_VertexArray;
		Ref<VertexBuffer> m_VertexBuffer;
		Ref<Material> m_Material;
		glm::vec2 m_ViewportSize;
	};
}