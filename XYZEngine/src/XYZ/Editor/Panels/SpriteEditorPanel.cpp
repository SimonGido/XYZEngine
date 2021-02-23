#include "stdafx.h"
#include "SpriteEditorPanel.h"

#include "XYZ/Core/Input.h"
#include "XYZ/InGui/InGui.h"
#include "XYZ/Renderer/Renderer2D.h"

#include "mapbox/earcut.hpp"

namespace XYZ {
	SpriteEditorPanel::SpriteEditorPanel(uint32_t panelID)
		:
		m_PanelID(panelID)
	{
		InGui::Begin(m_PanelID, "Sprite Editor", glm::vec2(0.0f), glm::vec2(200.0f));
		InGui::End();
	}
	void SpriteEditorPanel::SetContext(Ref<SubTexture> context)
	{
		m_Context = context;
		m_Size.x = (float)m_Context->GetTexture()->GetWidth();
		m_Size.y = (float)m_Context->GetTexture()->GetHeight();
		m_Points.resize(1);
	}
	void SpriteEditorPanel::OnInGuiRender()
	{
		if (InGui::Begin(m_PanelID, "Sprite Editor", glm::vec2(0.0f), glm::vec2(200.0f)))
		{
			if (m_Context.Raw())
			{
				glm::vec2 windowSize = InGui::GetWindow(m_PanelID).Size;
				glm::vec2 windowPosition = InGui::GetWindow(m_PanelID).Position;
				glm::vec2 position = (windowSize / 2.0f) - (m_Size / 2.0f);
				
				glm::vec2 nextPos = InGui::GetPositionOfNext();
				InGui::SetPositionOfNext(windowPosition + position);
				InGui::Image(m_Size, m_Context);
				for (auto& point : m_Points[0])
				{
					Renderer2D::SubmitCircle(glm::vec3(windowPosition.x + point[0], windowPosition.y + point[1], 0.0f), 5.0f, 20, glm::vec4(1.0f, 0.0f, 1.0f, 1.0f));
				}
				for (size_t i = 1; i < m_Indices.size(); ++i)
				{
					uint32_t previousIndex = m_Indices[i - 1];
					uint32_t currentIndex = m_Indices[i];
					Point& previous = m_Points[0][previousIndex];
					Point& current = m_Points[0][currentIndex];
					Renderer2D::SubmitLine(glm::vec3(previous[0], previous[1], 0.0f), glm::vec3(current[0], current[1], 0.0f), glm::vec4(0.0f, 0.0f, 1.0f, 1.0f));
				}
				InGui::SetPositionOfNext(nextPos);
				if (IS_SET(InGui::Button("Triangulate", glm::vec2(70.0f, 50.0f)), InGuiReturnType::Clicked))
				{
					m_Indices = mapbox::earcut<uint32_t>(m_Points);
				}
			}
		}
		InGui::End();
	}
	void SpriteEditorPanel::OnEvent(Event& event)
	{
		EventDispatcher dispatcher(event);
		dispatcher.Dispatch<MouseButtonPressEvent>(Hook(&SpriteEditorPanel::onMouseButtonPress, this));
	}
	bool SpriteEditorPanel::onMouseButtonPress(MouseButtonPressEvent& event)
	{
		if (event.IsButtonPressed(MouseCode::MOUSE_BUTTON_RIGHT))
		{
			if (IS_SET(InGui::GetWindow(m_PanelID).Flags, InGuiWindowFlags::Hoovered))
			{
				auto [mx, my] = Input::GetMousePosition();
				glm::vec2 windowPosition = InGui::GetWindow(m_PanelID).Position;
				glm::vec2 relativePos = glm::vec2( mx, my ) - windowPosition;

				m_Points[0].push_back({ relativePos.x, relativePos.y });

				return true;
			}
		}

		return false;
	}
}