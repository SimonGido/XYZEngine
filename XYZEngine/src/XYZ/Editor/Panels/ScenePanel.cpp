#include "stdafx.h"
#include "ScenePanel.h"

#include "XYZ/Core/Input.h"
#include "XYZ/Core/Application.h"
#include "XYZ/Scene/SceneEntity.h"
#include "XYZ/Renderer/SceneRenderer.h"

#include <imgui.h>

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
			//mx -= m_Window->Position.x;
			//my -= m_Window->Position.y;

			//auto viewportWidth = m_Window->Size.x;
			//auto viewportHeight = m_Window->Size.y;
			//
			//return { (mx / viewportWidth) * 2.0f - 1.0f, ((my / viewportHeight) * 2.0f - 1.0f) * -1.0f };
			return std::pair<float, float>();
		}

		ScenePanel::ScenePanel()
			:
			m_ViewportSize(0.0f),
			m_EditorCamera(30.0f, 1.778f, 0.1f, 1000.0f),
			m_ViewportFocused(false),
			m_ViewportHovered(false)
		{
			uint32_t windowWidth = Application::Get().GetWindow().GetWidth();
			uint32_t windowHeight = Application::Get().GetWindow().GetHeight();
			m_EditorCamera.SetViewportSize((float)windowWidth, (float)windowHeight);
		}
		void ScenePanel::SetContext(Ref<Scene> context)
		{
			m_Context = context;
		}

		void ScenePanel::OnUpdate(Timestep ts)
		{
			if (m_ViewportHovered)
			{
				m_EditorCamera.OnUpdate(ts);
			}
		}	
		void ScenePanel::OnImGuiRender()
		{
			if (ImGui::Begin("Scene"))
			{
				auto viewportMinRegion = ImGui::GetWindowContentRegionMin();
				auto viewportMaxRegion = ImGui::GetWindowContentRegionMax();
				auto viewportOffset = ImGui::GetWindowPos();
				m_ViewportBounds[0] = { viewportMinRegion.x + viewportOffset.x, viewportMinRegion.y + viewportOffset.y };
				m_ViewportBounds[1] = { viewportMaxRegion.x + viewportOffset.x, viewportMaxRegion.y + viewportOffset.y };

				m_ViewportFocused = ImGui::IsWindowFocused();
				m_ViewportHovered = ImGui::IsWindowHovered();
				Application::Get().GetImGuiLayer()->BlockEvents(!m_ViewportFocused && !m_ViewportHovered);

				ImVec2 viewportPanelSize = ImGui::GetContentRegionAvail();
				if (m_ViewportSize.x != viewportPanelSize.x || m_ViewportSize.y != viewportPanelSize.y)
				{
					m_ViewportSize = { viewportPanelSize.x, viewportPanelSize.y };
					SceneRenderer::SetViewportSize((uint32_t)m_ViewportSize.x, (uint32_t)m_ViewportSize.y);
					m_EditorCamera.SetViewportSize(m_ViewportSize.x, m_ViewportSize.y);
					m_Context->SetViewportSize((uint32_t)m_ViewportSize.x, (uint32_t)m_ViewportSize.y);
				}

				ImGui::Image(reinterpret_cast<void*>(SceneRenderer::GetFinalColorBufferRendererID()), ImVec2{ m_ViewportSize.x, m_ViewportSize.y }, ImVec2{ 0, 1 }, ImVec2{ 1, 0 });
			}
			ImGui::End();
		}
	}
}