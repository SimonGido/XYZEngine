#include "stdafx.h"
#include "ScenePanel.h"

#include "XYZ/Core/Input.h"
#include "XYZ/Core/Application.h"
#include "XYZ/Scene/SceneEntity.h"
#include "XYZ/Renderer/SceneRenderer.h"


namespace XYZ {
	namespace Editor {
		std::pair<glm::vec3, glm::vec3> ScenePanel::castRay(float mx, float my) const
		{
			glm::vec4 mouseClipPos = { mx, my, -1.0f, 1.0f };

			auto inverseProj = glm::inverse(m_EditorCamera.GetProjectionMatrix());
			auto inverseView = glm::inverse(glm::mat3(m_EditorCamera.GetViewMatrix()));

			glm::vec4 ray = inverseProj * mouseClipPos;
			glm::vec3 rayPos = m_EditorCamera.GetPosition();
			glm::vec3 rayDir = inverseView * glm::vec3(ray);

			return { rayPos, rayDir };
		}

		std::pair<float, float> ScenePanel::getMouseViewportSpace() const
		{
			auto [mx, my] = Input::GetMousePosition();
			mx -= m_Window->Position.x;
			my -= m_Window->Position.y;

			auto viewportWidth = m_Window->Size.x;
			auto viewportHeight = m_Window->Size.y;

			return { (mx / viewportWidth) * 2.0f - 1.0f, ((my / viewportHeight) * 2.0f - 1.0f) * -1.0f };
			return std::pair<float, float>();
		}

		ScenePanel::ScenePanel()
			:
			m_ViewportSize(0.0f),
			m_EditorCamera(30.0f, 1.778f, 0.1f, 1000.0f),
			m_Window(nullptr)
		{
			uint32_t windowWidth = Application::Get().GetWindow().GetWidth();
			uint32_t windowHeight = Application::Get().GetWindow().GetHeight();
			m_EditorCamera.SetViewportSize((float)windowWidth, (float)windowHeight);
		}
		void ScenePanel::SetContext(Ref<Scene> context)
		{
			m_Context = context;
		}
		void ScenePanel::SetSubTexture(Ref<SubTexture> subTexture)
		{
			m_SceneSubTexture = subTexture;
		}

		void ScenePanel::OnUpdate(Timestep ts)
		{
			InGuiConfig& config = InGui::GetContext().m_Config;
			glm::vec2 oldPadding = config.WindowPadding;
			glm::vec4 oldHighlightColor = config.Colors[InGuiConfig::ImageHighlight];
			config.WindowPadding = glm::vec2(0.0f);
			config.Colors[InGuiConfig::ImageHighlight] = glm::vec4(1.0f);

			if (InGui::Begin("Scene"))
			{
				m_Window = InGui::GetContext().GetInGuiWindow("Scene");
				if (m_Window->Size != m_ViewportSize)
				{
					m_ViewportSize = m_Window->Size;
					m_EditorCamera.SetViewportSize(m_ViewportSize.x, m_ViewportSize.y);
					SceneRenderer::SetViewportSize((uint32_t)m_ViewportSize.x, (uint32_t)m_ViewportSize.y);
				}		
				InGui::Image("#####", m_Window->Size, m_SceneSubTexture);
			}
			InGui::End();
			config.WindowPadding = oldPadding;
			config.Colors[InGuiConfig::ImageHighlight] = oldHighlightColor;
			Application::Get().GetInGuiLayer()->BlockEvents(!IS_SET(m_Window->EditFlags, InGuiWindowEditFlags::Hoovered));
		}

		void ScenePanel::OnEvent(Event& event)
		{
			if (m_Context.Raw())
			{
		
			}
		}
		bool ScenePanel::onWindowResize(WindowResizeEvent& event)
		{
			return false;
		}
		bool ScenePanel::onMouseButtonPress(MouseButtonPressEvent& event)
		{
			if (event.IsButtonPressed(MouseCode::MOUSE_BUTTON_LEFT) && !Input::IsKeyPressed(KeyCode::KEY_LEFT_ALT))
			{
				
			}
			return false;
		}
		bool ScenePanel::onKeyPress(KeyPressedEvent& event)
		{
			if (m_Context->GetSelectedEntity().IsValid())
			{
				if (m_ModifyFlags)
				{
					if (event.IsKeyPressed(KeyCode::KEY_X))
					{
						m_ModifyFlags |= ModifyFlags::X;
						m_ModifyFlags &= ~(ModifyFlags::Y | ModifyFlags::Z);
					}
					else if (event.IsKeyPressed(KeyCode::KEY_Y))
					{
						m_ModifyFlags |= ModifyFlags::Y;
						m_ModifyFlags &= ~(ModifyFlags::X | ModifyFlags::Z);
					}
					else if (event.IsKeyPressed(KeyCode::KEY_Z))
					{
						m_ModifyFlags |= ModifyFlags::Z;
						m_ModifyFlags &= ~(ModifyFlags::Y | ModifyFlags::X);
					}
				}
				else
				{
					m_ModifyFlags = 0;
				}

				if (event.IsKeyPressed(KeyCode::KEY_G))
				{
					m_ModifyFlags = ModifyFlags::Move;
				}
				else if (event.IsKeyPressed(KeyCode::KEY_R))
				{
					m_ModifyFlags = ModifyFlags::Rotate;
				}
				else if (event.IsKeyPressed(KeyCode::KEY_S))
				{
					m_ModifyFlags = ModifyFlags::Scale;
				}

				if (m_ModifyFlags) m_OldMousePosition = { Input::GetMouseX(), Input::GetMouseY() };
				return m_ModifyFlags;
			}
			m_ModifyFlags = 0;
			return false;
		}
		bool ScenePanel::onKeyRelease(KeyReleasedEvent& event)
		{
			return false;
		}
	}
}