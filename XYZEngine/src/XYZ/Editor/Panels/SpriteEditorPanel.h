#pragma once
#include "XYZ/Renderer/SubTexture.h"
#include "XYZ/Event/Event.h"
#include "XYZ/Event/InputEvent.h"

#include <array>

namespace XYZ {
	using Point = std::array<float, 2>;

	class SpriteEditorPanel
	{
	public:
		SpriteEditorPanel(uint32_t panelID);

		void SetContext(Ref<SubTexture> context);

		void OnInGuiRender();
		void OnEvent(Event& event);

	private:
		bool onMouseButtonPress(MouseButtonPressEvent& event);


	private:
		const uint32_t m_PanelID;

		Ref<SubTexture> m_Context;

		glm::vec2 m_Size = glm::vec2(0.0f);

		
		std::vector<std::vector<Point>> m_Points;
		std::vector<uint32_t> m_Indices;
	};
}