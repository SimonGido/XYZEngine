#include "stdafx.h"
#include "InGuiContext.h"

#include "InGui.h"
#include "InGuiBehavior.h"
#include "InGuiUtil.h"

#include "XYZ/Core/Application.h"
#include "XYZ/Renderer/Renderer2D.h"

#include <glm/gtx/transform.hpp>

#include <stack>

namespace XYZ {

	InGuiContext::InGuiContext()
		:
		m_ViewportWidth(0),
		m_ViewportHeight(0),
		m_FocusedWindow(nullptr),
		m_WindowPool(15 * sizeof(InGuiWindow)),
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

	InGuiContext::~InGuiContext()
	{
		for (auto window : m_Windows)
			m_WindowPool.Deallocate<InGuiWindow>(window);
	}

	void InGuiContext::Render()
	{
		handleWindowCursor();
		for (InGuiWindow* window : m_Windows)
		{
			m_ClipRectangles[window->ClipID] = window->ClipRect(m_ViewportHeight);
			m_ClipRectangles[window->PanelClipID] = window->PanelClipRect(m_ViewportHeight);
		}
		glm::mat4 viewMatrix = glm::translate(glm::mat4(1.0f), glm::vec3(0.0f));
		glm::mat4 viewProjectionMatrix = glm::ortho(0.0f, (float)m_ViewportWidth, (float)m_ViewportHeight, 0.0f) * glm::inverse(viewMatrix);
		
		Renderer2D::BeginScene(viewProjectionMatrix);
		CustomRenderer2D::BeginScene(m_RendererLayout);
		CustomRenderer2D::SetMaterial(m_Config.Material);
		CustomRenderer2D::SetLineShader(m_Config.LineShader);
		for (const auto& texture : m_FrameData.CustomTextures)
			CustomRenderer2D::SetTexture(texture);

		m_ClipBuffer->Update(m_ClipRectangles.data(), m_ClipRectangles.size() * sizeof(InGuiRect));
		m_ClipBuffer->BindRange(0, m_ClipRectangles.size() * sizeof(InGuiRect), 0);

		for (InGuiWindow* window : m_Windows)
			window->DrawList.SubmitToRenderer();

		if (m_DockSpace.IsInitialized())
			m_DockSpace.Drawlist.SubmitToRenderer();

		CustomRenderer2D::Flush();
		CustomRenderer2D::FlushLines();
		CustomRenderer2D::EndScene();
		m_FrameData.CustomTextures.clear();
	}

	void InGuiContext::SetViewportSize(uint32_t width, uint32_t height)
	{
		m_ViewportWidth = width;
		m_ViewportHeight = height;
		m_ClipRectangles[0] = { glm::vec2(0.0f), glm::vec2((float)width, (float)height) };
		m_DockSpace.Resize({ m_ViewportWidth, m_ViewportHeight });
	}

	void InGuiContext::FocusWindow(InGuiWindow* window)
	{
		auto it = std::find(m_Windows.begin(), m_Windows.end(), window);
		InGuiWindow* tmp = m_Windows.back();
		m_Windows.back() = *it;
		*it = tmp;
		m_FocusedWindow = window;
		m_FocusedWindow->EditFlags |= InGuiWindowEditFlags::Modified;
	}

	void InGuiContext::OnEvent(Event& event)
	{
		EventDispatcher dispatcher(event);
		dispatcher.Dispatch<MouseButtonPressEvent>([&](MouseButtonPressEvent& e)->bool {
			if (e.IsButtonPressed(MouseCode::MOUSE_BUTTON_LEFT))
			{
				m_FocusedWindow = nullptr;
				m_Input.Flags |= InGuiInput::LeftMouseButtonPressed;
				if (m_DockSpace.IsInitialized())
				{
					if (m_DockSpace.FindResizedNode(m_Input.MousePosition))
						return true;
				}
				for (auto it = m_Windows.rbegin(); it != m_Windows.rend(); ++it)
				{
					InGuiWindow* window = (*it);
					for (auto child : window->ChildWindows)
					{
						if (handleWindowLeftButtonPress(child))
							return IS_SET(child->EditFlags, InGuiWindowEditFlags::BlockEvents);
					}
					if (handleWindowLeftButtonPress(window))
						return IS_SET(window->EditFlags, InGuiWindowEditFlags::BlockEvents);
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
				if (m_DockSpace.IsInitialized())
				{
					m_DockSpace.ResizedNode = nullptr;
					if (m_FocusedWindow && IS_SET(m_FocusedWindow->StyleFlags, InGuiWindowStyleFlags::DockingEnabled))
						m_DockSpace.InsertWindow(m_FocusedWindow);
				}
			}
			else if (e.IsButtonReleased(MouseCode::MOUSE_BUTTON_RIGHT))
			{
				m_Input.Flags &= ~InGuiInput::RightMouseButtonPressed;
			}
			return false;
		});

		dispatcher.Dispatch<MouseScrollEvent>([&](MouseScrollEvent& e)->bool {

			if (
				m_FocusedWindow 
				&& m_FocusedWindow->ClipRect().Overlaps(m_Input.MousePosition)
				&& IS_SET(m_FocusedWindow->StyleFlags, InGuiWindowStyleFlags::ScrollEnabled)
			)
			{
				if (m_FocusedWindow->ScrollBarX)
				{
					float newScrollX = m_FocusedWindow->Scroll.x + e.GetOffsetX();
					if (newScrollX <= m_FocusedWindow->FrameData.ScrollMax.x
						&& newScrollX >= 0.0f)
					{
						m_FocusedWindow->Scroll.x = newScrollX;
					}
				}
				if (m_FocusedWindow->ScrollBarY)
				{
					float newScrollY = m_FocusedWindow->Scroll.y - e.GetOffsetY();
					if (newScrollY <= m_FocusedWindow->FrameData.ScrollMax.y
						&& newScrollY >= 0.0f)
					{
						m_FocusedWindow->Scroll.y = newScrollY;
					}
					return true;
				}
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

	InGuiWindow* InGuiContext::CreateInGuiWindow(const char* name)
	{
		InGuiClipID workClipID = m_ClipRectangles.size();
		InGuiClipID panelClipID = m_ClipRectangles.size() + 1;
		InGuiWindow* window = m_WindowPool.Allocate<InGuiWindow>(workClipID, panelClipID);
		window->Name = name;
		m_Windows.push_back(window);

		std::hash<std::string_view> hasher;
		InGuiID id = hasher(name);
		m_WindowMap[id] = window;
		m_ClipRectangles.push_back({});
		m_ClipRectangles.push_back({});
		return window;
	}
	InGuiWindow* InGuiContext::GetInGuiWindow(const char* name)
	{
		std::hash<std::string_view> hasher;
		InGuiID id = hasher(name);
		auto it = m_WindowMap.find(id);
		if (it == m_WindowMap.end())
			return nullptr;
		it->second->IsInitialized = true;
		return it->second;
	}
	bool InGuiContext::handleWindowLeftButtonPress(InGuiWindow* window)
	{
		InGuiRect windowRect = window->RealRect();
		if (window->IsActive && window->IsInitialized && windowRect.Overlaps(m_Input.MousePosition))
		{
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
			}
			else
			{
				result = 0;
				InGuiBehavior::ButtonBehavior(panelRect, id, result);
				if (IS_SET(result, InGui::Pressed))
				{
					window->EditFlags |= InGuiWindowEditFlags::Moving;
					if (window->DockNode)
						window->DockNode->RemoveWindow(window);
					m_FrameData.MovedWindowOffset = m_Input.MousePosition - window->Position;
				}
				else if (window->ResolveResizeFlags(m_Input.MousePosition))
				{
				}
			}
			// Unset current window before end of the function
			m_FrameData.CurrentWindow = nullptr;
			return true;
		}
		return false;
	}
	void InGuiContext::handleWindowCursor()
	{
		auto& window = Application::Get().GetWindow();
		const glm::vec2 mousePosition = m_Input.MousePosition;
		float threshhold = m_Config.ResizeThreshhold;
		for (auto win : m_Windows)
		{			
			if (win->RealRect().Overlaps(mousePosition))
			{
				if (mousePosition.x < win->Position.x + threshhold)
				{
					window.SetStandardCursor(WindowCursors::XYZ_HRESIZE_CURSOR);
					return;
				}
				else if (mousePosition.x > win->Position.x + win->Size.x - threshhold)
				{
					window.SetStandardCursor(WindowCursors::XYZ_HRESIZE_CURSOR);
					return;
				}
				else if (mousePosition.y > win->Position.y + win->Size.y - threshhold)
				{
					window.SetStandardCursor(WindowCursors::XYZ_VRESIZE_CURSOR);
					return;
				}
			}
		}
		if (m_DockSpace.ResizedNode)
		{
			if (m_DockSpace.ResizedNode->Split == SplitType::Horizontal)
			{
				window.SetStandardCursor(WindowCursors::XYZ_VRESIZE_CURSOR);
				return;
			}
			else
			{
				window.SetStandardCursor(WindowCursors::XYZ_HRESIZE_CURSOR);
				return;
			}
		}
		else if (m_FocusedWindow)
		{
			if (IS_SET(m_FocusedWindow->EditFlags, InGuiWindowEditFlags::ResizeLeft | InGuiWindowEditFlags::ResizeRight))
			{
				window.SetStandardCursor(WindowCursors::XYZ_HRESIZE_CURSOR);
				return;
			}
			else if (IS_SET(m_FocusedWindow->EditFlags, InGuiWindowEditFlags::ResizeBottom))
			{
				window.SetStandardCursor(WindowCursors::XYZ_VRESIZE_CURSOR);
				return;
			}
		}
		window.SetStandardCursor(WindowCursors::XYZ_ARROW_CURSOR);
	}
}