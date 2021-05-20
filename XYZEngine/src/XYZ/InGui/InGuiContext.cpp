#include "stdafx.h"
#include "InGuiContext.h"

#include "InGui.h"
#include "InGuiBehavior.h"

#include "XYZ/Renderer/Renderer2D.h"

#include <glm/gtx/transform.hpp>

namespace XYZ {

	InGuiContext::InGuiContext()
		:
		m_ViewportWidth(0),
		m_ViewportHeight(0),
		m_FocusedWindow(nullptr),
		m_Pool(15 * sizeof(InGuiWindow)),
		m_MenuBarActive(false),
		m_MenuOpenID(0)
	{
		m_RendererLayout.SetQuadBufferLayout(
			{
				{0, ShaderDataComponent::Float4, "a_Color" },
				{1, ShaderDataComponent::Float3, "a_Position" },
				{2, ShaderDataComponent::Float2, "a_TexCoord" },
				{3, ShaderDataComponent::Float,  "a_TextureID" },
				{4, ShaderDataComponent::Float,  "a_ScissorIndex" }
			}
		);
		m_RendererLayout.SetLineBufferLayout(
			{
			{0, ShaderDataComponent::Float3, "a_Position" },
			{1, ShaderDataComponent::Float4, "a_Color" },
			{2, ShaderDataComponent::Float,  "a_ScissorIndex" }
			}
		);

		m_ClipBuffer = ShaderStorageBuffer::Create(sc_MaxNumberOfClipRectangles * sizeof(InGuiRect));
		m_ClipRectangles.push_back({});
	}

	void InGuiContext::Render()
	{
		for (InGuiWindow* window : m_Windows)
			m_ClipRectangles[window->ClipID] = window->ClipRect(m_ViewportHeight);

		glm::mat4 viewMatrix = glm::translate(glm::mat4(1.0f), glm::vec3(0.0f));
		glm::mat4 viewProjectionMatrix = glm::ortho(0.0f, (float)m_ViewportWidth, (float)m_ViewportHeight, 0.0f) * glm::inverse(viewMatrix);
		
		Renderer2D::BeginScene(viewProjectionMatrix);
		CustomRenderer2D::BeginScene(m_RendererLayout);
		CustomRenderer2D::SetMaterial(m_Config.Material);
		CustomRenderer2D::SetLineShader(m_Config.LineShader);

		m_ClipBuffer->Update(m_ClipRectangles.data(), m_ClipRectangles.size() * sizeof(InGuiRect));
		m_ClipBuffer->BindRange(0, m_ClipRectangles.size() * sizeof(InGuiRect), 0);

		for (InGuiWindow* window : m_Windows)
			window->DrawList.SubmitToRenderer();

		

		CustomRenderer2D::Flush();
		CustomRenderer2D::FlushLines();
		CustomRenderer2D::EndScene();
	}

	void InGuiContext::SetViewportSize(uint32_t width, uint32_t height)
	{
		m_ViewportWidth = width;
		m_ViewportHeight = height;
		m_ClipRectangles[0] = { glm::vec2(0.0f), glm::vec2((float)width, (float)height) };
	}

	void InGuiContext::FocusWindow(InGuiWindow* window)
	{
		auto it = std::find(m_Windows.begin(), m_Windows.end(), window);
		InGuiWindow* tmp = m_Windows.back();
		m_Windows.back() = *it;
		*it = tmp;
		m_FocusedWindow = window;
	}

	void InGuiContext::OnEvent(Event& event)
	{
		EventDispatcher dispatcher(event);
		dispatcher.Dispatch<MouseButtonPressEvent>([&](MouseButtonPressEvent& e)->bool {
			if (e.IsButtonPressed(MouseCode::MOUSE_BUTTON_LEFT))
			{
				m_Input.Flags |= InGuiInput::LeftMouseButtonPressed;
				for (auto it = m_Windows.rbegin(); it != m_Windows.rend(); ++it)
				{
					InGuiWindow* window = (*it);
					InGuiRect windowRect = window->Rect();
					if (windowRect.Overlaps(m_Input.MousePosition))
					{					
						bool handled = false;
						FocusWindow(window);
						// Set current window so ButtonBehavior works correctly
						m_FrameData.CurrentWindow = window;

						InGuiRect panelRect = window->PanelRect();
						InGuiRect minimizeRect = window->MinimizeRect();

						uint8_t result = 0;
						std::hash<std::string_view> hasher;
						InGuiID id = hasher(window->Name.c_str());
						InGuiBehavior::ButtonBehavior(minimizeRect, id, result);
						if (IS_SET(result, InGui::Pressed))
						{
							window->EditFlags ^= InGuiWindowEditFlags::Collapsed;
							handled = IS_SET(window->EditFlags, InGuiWindowEditFlags::BlockEvents);
						}
						else
						{
							result = 0;
							InGuiBehavior::ButtonBehavior(panelRect, id, result);
							if (IS_SET(result, InGui::Pressed))
							{
								window->EditFlags |= InGuiWindowEditFlags::Moving;
								m_FrameData.MovedWindowOffset = m_Input.MousePosition - window->Position;
								handled = IS_SET(window->EditFlags, InGuiWindowEditFlags::BlockEvents);
							}
							else if (window->ResolveResizeFlags(m_Input.MousePosition))
							{							
								handled = IS_SET(window->EditFlags, InGuiWindowEditFlags::BlockEvents);
							}
						}			
						// Unset current window before end of the function
						m_FrameData.CurrentWindow = nullptr;
						return handled;
					}
				}
			}
			else if (e.IsButtonPressed(MouseCode::MOUSE_BUTTON_RIGHT))
			{
				m_Input.Flags |= InGuiInput::RightMouseButtonPressed;
			}
			return false;
		});

		dispatcher.Dispatch<MouseButtonReleaseEvent>([&](MouseButtonReleaseEvent& e)->bool {
			if (e.IsButtonReleased(MouseCode::MOUSE_BUTTON_LEFT))
			{
				m_Input.Flags &= ~InGuiInput::LeftMouseButtonPressed;
				m_LastLeftPressedID = 0;
				if (m_FocusedWindow)
				{
					m_FocusedWindow->EditFlags &= ~(
						  InGuiWindowEditFlags::Moving
						| InGuiWindowEditFlags::ResizeRight
						| InGuiWindowEditFlags::ResizeLeft
						| InGuiWindowEditFlags::ResizeBottom
						);
				}
			}
			else if (e.IsButtonReleased(MouseCode::MOUSE_BUTTON_RIGHT))
			{
				m_Input.Flags &= ~InGuiInput::RightMouseButtonPressed;
			}
			return false;
		});

		dispatcher.Dispatch<MouseScrollEvent>([&](MouseScrollEvent& e)->bool {

			if (m_FocusedWindow && m_FocusedWindow->ClipRect().Overlaps(m_Input.MousePosition))
			{
				float newScrollX = m_FocusedWindow->Scroll.x + e.GetOffsetX();
				float newScrollY = m_FocusedWindow->Scroll.y - e.GetOffsetY();
				if (newScrollX <= m_FocusedWindow->FrameData.ScrollMax.x
				&&  newScrollX >= 0.0f)
				{
					m_FocusedWindow->Scroll.x = newScrollX;
				}
				if (newScrollY <= m_FocusedWindow->FrameData.ScrollMax.y
				&&  newScrollY >= 0.0f)
				{
					m_FocusedWindow->Scroll.y = newScrollY;
				}
				return true;
			}
			return false;
		});

		dispatcher.Dispatch<MouseMovedEvent>([&](MouseMovedEvent& e)->bool {

			m_LastHooveredID = 0;
			return false;
		});

		dispatcher.Dispatch<KeyPressedEvent>([&](KeyPressedEvent& e)->bool {

			m_Input.KeyPressed = e.GetKey();
			return m_LastInputID;
		});

		dispatcher.Dispatch<KeyTypedEvent>([&](KeyTypedEvent& e)->bool {
			m_Input.KeyTyped = e.GetKey();
			return m_LastInputID;
		});
	}

	InGuiWindow* InGuiContext::CreateWindow(const char* name)
	{
		InGuiWindow* window = m_Pool.Allocate<InGuiWindow>();
		window->Name = name;
		m_Windows.push_back(window);
		window->ClipID = m_Windows.size();
		m_WindowMap[name] = window;
		m_ClipRectangles.push_back({});
		return window;
	}
	InGuiWindow* InGuiContext::GetWindow(const char* name)
	{
		auto it = m_WindowMap.find(name);
		if (it == m_WindowMap.end())
			return nullptr;
		return it->second;
	}
}