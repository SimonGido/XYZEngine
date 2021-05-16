#include "stdafx.h"
#include "InGuiContext.h"

#include "XYZ/Renderer/Renderer2D.h"

#include <glm/gtx/transform.hpp>

namespace XYZ {

	InGuiContext::InGuiContext()
		:
		m_ViewportWidth(0),
		m_ViewportHeight(0),
		m_FocusedWindow(nullptr),
		m_Pool(15 * sizeof(InGuiWindow))
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
			m_ClipRectangles[window->ID] = window->ClipRect(m_ViewportHeight);

		glm::mat4 viewMatrix = glm::translate(glm::mat4(1.0f), glm::vec3(0.0f));
		glm::mat4 viewProjectionMatrix = glm::ortho(0.0f, (float)m_ViewportWidth, (float)m_ViewportHeight, 0.0f) * glm::inverse(viewMatrix);
		
		Renderer2D::BeginScene(viewProjectionMatrix);
		CustomRenderer2D::BeginScene(m_RendererLayout);
		CustomRenderer2D::SetMaterial(m_Config.m_Material);
		CustomRenderer2D::SetLineShader(m_Config.m_LineShader);

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
				for (auto it = m_Windows.rbegin(); it != m_Windows.rend(); ++it)
				{
					InGuiWindow* window = (*it);
					InGuiRect windowRect = window->Rect();
					if (windowRect.Overlaps(m_FrameData.MousePosition))
					{
						InGuiRect panelRect = window->PanelRect();
						InGuiRect minimizeRect = window->MinimizeRect();
						
						if (minimizeRect.Overlaps(m_FrameData.MousePosition))
						{
							window->Flags ^= InGuiWindow::Collapsed;
							return IS_SET(window->Flags, InGuiWindow::BlockEvents);
						}
						else if (panelRect.Overlaps(m_FrameData.MousePosition))
						{
							FocusWindow(window);
							window->Flags |= InGuiWindow::Moving;
							m_FrameData.MovedWindowOffset = m_FrameData.MousePosition - window->Position;
							return IS_SET(window->Flags, InGuiWindow::BlockEvents);
						}
						else if (window->ResolveResizeFlags(m_FrameData.MousePosition))
							return IS_SET(window->Flags, InGuiWindow::BlockEvents);
					}
				}
			}
			return false;
		});

		dispatcher.Dispatch<MouseButtonReleaseEvent>([&](MouseButtonReleaseEvent& e)->bool {
			if (e.IsButtonReleased(MouseCode::MOUSE_BUTTON_LEFT))
			{
				if (m_FocusedWindow)
				{
					m_FocusedWindow->Flags &= ~InGuiWindow::Moving;
					m_FocusedWindow = nullptr;
				}
			}
			return false;
		});
	}

	InGuiWindow* InGuiContext::CreateWindow(const char* name)
	{
		InGuiWindow* window = m_Pool.Allocate<InGuiWindow>();
		window->Name = name;
		m_Windows.push_back(window);
		window->ID = m_Windows.size();
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