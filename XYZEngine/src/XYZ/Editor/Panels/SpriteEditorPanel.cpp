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

		auto flags = InGui::GetWindow(m_PanelID).Flags;
		flags &= ~InGuiWindowFlags::EventBlocking;
		InGui::SetWindowFlags(m_PanelID, flags);
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
				auto& layout = InGui::GetWindow(m_PanelID).Layout;
				
				InGui::BeginScrollableArea(windowSize - glm::vec2(2.0f * layout.RightPadding, 0.0f), m_ScrollOffset, 100.0f, 10.0f);
				glm::vec2 nextPos = InGui::GetPositionOfNext();
				InGui::SetPositionOfNext(windowPosition + position);
				InGui::Image(m_Size * 2.0f, m_Context);
				
				for (auto& point : m_Points[0])
				{
					glm::vec2 relativePos = {
						windowPosition.x + (windowSize.x / 2.0f) + point[0],
						windowPosition.y + (windowSize.y / 2.0f) + point[1]
					};
					Renderer2D::SubmitCircle(glm::vec3(relativePos.x, relativePos.y, 0.0f), sc_PointRadius, 20, glm::vec4(1.0f, 0.0f, 1.0f, 1.0f));
				}
				for (size_t i = 1; i < m_Indices.size(); ++i)
				{
					uint32_t previousIndex = m_Indices[i - 1];
					uint32_t currentIndex = m_Indices[i];
					Point& previous = m_Points[0][previousIndex];
					Point& current = m_Points[0][currentIndex];

					glm::vec2 previousRelPos = {
						windowPosition.x + (windowSize.x / 2.0f) + previous[0],
						windowPosition.y + (windowSize.y / 2.0f) + previous[1]
					};
					glm::vec2 currentRelPos = {
						windowPosition.x + (windowSize.x / 2.0f) + current[0],
						windowPosition.y + (windowSize.y / 2.0f) + current[1]
					};
					Renderer2D::SubmitLine(
						glm::vec3(previousRelPos.x, previousRelPos.y, 0.0f), 
						glm::vec3(currentRelPos.x, currentRelPos.y, 0.0f), glm::vec4(0.0f, 0.0f, 1.0f, 1.0f));
				}
				InGui::SetPositionOfNext(nextPos);
				if (IS_SET(InGui::Button("Triangulate", glm::vec2(70.0f, 50.0f)), InGuiReturnType::Clicked))
				{
					m_Indices = mapbox::earcut<uint32_t>(m_Points);
				}
				if (m_MovedPoint)
				{
					auto [mx, my] = Input::GetMousePosition();
					glm::vec2 relativePos = glm::vec2(mx, my) - windowPosition - (windowSize / 2.0f);

					(*m_MovedPoint)[0] = relativePos.x;
					(*m_MovedPoint)[1] = relativePos.y;
				}
				if (!IS_SET(InGui::GetWindow(m_PanelID).Flags, InGuiWindowFlags::Hoovered))
				{
					m_MovedPoint = nullptr;
				}
				InGui::EndScrollableArea();

			}
		}
		InGui::End();
	}
	void SpriteEditorPanel::OnEvent(Event& event)
	{
		EventDispatcher dispatcher(event);
		dispatcher.Dispatch<MouseButtonPressEvent>(Hook(&SpriteEditorPanel::onMouseButtonPress, this));
		dispatcher.Dispatch<MouseButtonReleaseEvent>(Hook(&SpriteEditorPanel::onMouseButtonRelease, this));
		dispatcher.Dispatch<MouseScrollEvent>(Hook(&SpriteEditorPanel::onMouseScroll, this));
	}
	bool SpriteEditorPanel::onMouseButtonPress(MouseButtonPressEvent& event)
	{
		if (event.IsButtonPressed(MouseCode::MOUSE_BUTTON_RIGHT))
		{
			if (IS_SET(InGui::GetWindow(m_PanelID).Flags, InGuiWindowFlags::Hoovered))
			{
				auto [mx, my] = Input::GetMousePosition();
				glm::vec2 windowPosition = InGui::GetWindow(m_PanelID).Position;
				glm::vec2 windowSize = InGui::GetWindow(m_PanelID).Size;
				glm::vec2 relativePos = glm::vec2( mx, my ) - windowPosition - (windowSize / 2.0f);
				uint32_t counter = 0;
				for (auto& point : m_Points[0])
				{
					glm::vec2 relativePos = {
						windowPosition.x + (windowSize.x / 2.0f) + point[0],
						windowPosition.y + (windowSize.y / 2.0f) + point[1]
					};
					if (glm::distance(glm::vec2(mx, my), relativePos) < sc_PointRadius)
					{
						m_Points[0].erase(m_Points[0].begin() + counter);
						if (m_Indices.size())
							m_Indices = mapbox::earcut<uint32_t>(m_Points);
						return true;
					}
					counter++;
				}


				m_Points[0].push_back({ relativePos.x, relativePos.y });

				return true;
			}
		}
		else if (event.IsButtonPressed(MouseCode::MOUSE_BUTTON_LEFT))
		{
			auto [mx, my] = Input::GetMousePosition();
			glm::vec2 windowPosition = InGui::GetWindow(m_PanelID).Position;
			glm::vec2 windowSize = InGui::GetWindow(m_PanelID).Size;
			glm::vec2 relativePos = glm::vec2(mx, my) - windowPosition - (windowSize / 2.0f);
			uint32_t counter = 0;
			for (auto& point : m_Points[0])
			{
				glm::vec2 relativePos = {
					windowPosition.x + (windowSize.x / 2.0f) + point[0],
					windowPosition.y + (windowSize.y / 2.0f) + point[1]
				};
				if (glm::distance(glm::vec2(mx, my), relativePos) < sc_PointRadius)
				{
					m_MovedPoint = &point;
					return true;
				}
				counter++;
			}
		}
		return false;
	}
	bool SpriteEditorPanel::onMouseButtonRelease(MouseButtonReleaseEvent& event)
	{
		if (event.IsButtonReleased(MouseCode::MOUSE_BUTTON_LEFT))
		{
			m_MovedPoint = nullptr;
		}
		return false;
	}
	bool SpriteEditorPanel::onMouseScroll(MouseScrollEvent& event)
	{
		if (IS_SET(InGui::GetWindow(m_PanelID).Flags, InGuiWindowFlags::Hoovered))
		{
			//float yAllowedSize = InGui::GetWindow(m_PanelID).Size.y
			//	- InGui::GetWindow(m_PanelID).Layout.TopPadding
			//	- InGui::GetWindow(m_PanelID).Layout.BottomPadding;
			
			//if (m_Size.y * (m_Scale + event.GetOffsetY() * 0.1f) < yAllowedSize)
			{
				m_Scale += event.GetOffsetY() * 0.25f;
			}
		}
		return false;
	}
}