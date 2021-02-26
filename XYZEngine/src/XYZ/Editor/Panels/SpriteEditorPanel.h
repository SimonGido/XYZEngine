#pragma once
#include "XYZ/Renderer/SubTexture.h"
#include "XYZ/Event/Event.h"
#include "XYZ/Event/InputEvent.h"
#include "XYZ/InGui/InGui.h"

#include <array>

namespace XYZ {

	class SpriteEditorPanel
	{
	public:
		SpriteEditorPanel(uint32_t panelID);

		void SetContext(Ref<SubTexture> context);

		void OnInGuiRender();
		void OnEvent(Event& event);

	private:
		struct Point
		{
			float X, Y;
		};
		struct Triangle
		{
			uint32_t First = 0;
			uint32_t Second = 0;
			uint32_t Third = 0;
		};


	private:
		bool onMouseButtonPress(MouseButtonPressEvent& event);
		bool onMouseButtonRelease(MouseButtonReleaseEvent& event);
		bool onMouseScroll(MouseScrollEvent& event);

		void triangulate();
		void showTriangle(InGuiMesh& mesh, const Triangle& triangle, const glm::vec2& offset, const glm::vec4& color);
		Triangle findTriangle(const glm::vec2& pos);

	private:
		const uint32_t m_PanelID;

		Ref<SubTexture> m_Context;

		glm::vec2 m_Size = glm::vec2(0.0f);
		
		

		std::vector<Point> m_Points;
		std::vector<uint32_t> m_Indices;

		float m_Scale = 1.0f;
		float m_ScrollOffset = 0.0f;

		Point* m_MovedPoint = nullptr;
		Triangle m_Triangle;
		bool m_TriangleFound = false;

		bool m_Triangulated = false;

		static constexpr float sc_PointRadius = 5.0f;
	};
}