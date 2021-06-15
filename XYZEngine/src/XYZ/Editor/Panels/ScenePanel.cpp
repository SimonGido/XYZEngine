#include "stdafx.h"
#include "ScenePanel.h"

#include "XYZ/Core/Input.h"
#include "XYZ/Core/Application.h"
#include "XYZ/Scene/SceneEntity.h"
#include "XYZ/Renderer/SceneRenderer.h"
#include "XYZ/Renderer/Renderer2D.h"
#include "XYZ/Utils/Math/Ray.h"

#include <imgui.h>

namespace XYZ {
	namespace Editor {
		static glm::vec4 CalculateTexCoords(const glm::vec2& coords, const glm::vec2& size, const glm::vec2& textureSize)
		{
			return {
				(coords.x * size.x) / textureSize.x,
				((coords.y + 1) * size.y) / textureSize.y,
				((coords.x + 1) * size.x) / textureSize.x,
				(coords.y * size.y) / textureSize.y,
			};
		}

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
			mx -= ImGui::GetWindowPos().x;
			my -= ImGui::GetWindowPos().y;
		
			auto viewportWidth  = ImGui::GetWindowSize().x;
			auto viewportHeight = ImGui::GetWindowSize().y;
			
			return { (mx / viewportWidth) * 2.0f - 1.0f, ((my / viewportHeight) * 2.0f - 1.0f) * -1.0f };
			return std::pair<float, float>();
		}

		void ScenePanel::showSelection(SceneEntity entity)
		{
			if (entity.HasComponent<CameraComponent>())
			{
				auto& camera = entity.GetComponent<CameraComponent>().Camera;
				camera.SetViewportSize((uint32_t)m_ViewportSize.x, (uint32_t)m_ViewportSize.y);
				auto transformComponent = entity.GetComponent<TransformComponent>();

				auto& translation = transformComponent.Translation;
				if (camera.GetProjectionType() == CameraProjectionType::Orthographic)
				{
					float size = camera.GetOrthographicProperties().OrthographicSize;
					float aspect = (float)camera.GetViewportWidth()/ (float)camera.GetViewportHeight();
					float width = size * aspect;
					float height = size;

					glm::vec3 topLeft = { translation.x - width / 2.0f,translation.y + height / 2.0f, translation.z };
					glm::vec3 topRight = { translation.x + width / 2.0f,translation.y + height / 2.0f, translation.z };
					glm::vec3 bottomLeft = { translation.x - width / 2.0f,translation.y - height / 2.0f, translation.z };
					glm::vec3 bottomRight = { translation.x + width / 2.0f,translation.y - height / 2.0f, translation.z };

					Renderer2D::SubmitLine(topLeft, topRight);
					Renderer2D::SubmitLine(topRight, bottomRight);
					Renderer2D::SubmitLine(bottomRight, bottomLeft);
					Renderer2D::SubmitLine(bottomLeft, topLeft);
				}
			}
			else
			{
				auto& transformComponent = entity.GetComponent<TransformComponent>();
				auto [translation, rotation, scale] = transformComponent.GetWorldComponents();

				glm::vec3 topLeft = { translation.x - scale.x / 2,translation.y + scale.y / 2, translation.z };
				glm::vec3 topRight = { translation.x + scale.x / 2,translation.y + scale.y / 2, translation.z };
				glm::vec3 bottomLeft = { translation.x - scale.x / 2,translation.y - scale.y / 2, translation.z };
				glm::vec3 bottomRight = { translation.x + scale.x / 2,translation.y - scale.y / 2, translation.z };

				Renderer2D::SubmitLine(topLeft, topRight);
				Renderer2D::SubmitLine(topRight, bottomRight);
				Renderer2D::SubmitLine(bottomRight, bottomLeft);
				Renderer2D::SubmitLine(bottomLeft, topLeft);
			}
		}

		void ScenePanel::handleSelection(const glm::vec2& mousePosition)
		{
			if (ImGui::GetIO().MouseDown[ImGuiMouseButton_Left]
				&& !ImGui::GetIO().KeyAlt)
			{
				auto [origin, direction] = castRay(mousePosition.x, mousePosition.y);
				Ray ray = { origin,direction };
				m_Context->SetSelectedEntity(Entity());
				if (m_Callback)
					m_Callback(m_Context->GetSelectedEntity());

				for (Entity entityID : m_Context->GetEntities())
				{
					SceneEntity entity(entityID, m_Context.Raw());
					TransformComponent& transformComponent = entity.GetComponent<TransformComponent>();
					auto [translation, rotation, scale] = transformComponent.GetWorldComponents();
					AABB aabb(
						translation - (scale / 2.0f),
						translation + (scale / 2.0f)
					);

					if (ray.IntersectsAABB(aabb))
					{
						m_Context->SetSelectedEntity(entityID);
						if (m_Callback)
							m_Callback(m_Context->GetSelectedEntity());
					}
				}
			}
		}

		ScenePanel::ScenePanel()
			:
			m_ViewportSize(0.0f),
			m_ViewportBounds{ {},{} },
			m_ButtonSize(25.0f),
			m_EditorCamera(30.0f, 1.778f, 0.1f, 1000.0f),
			m_ViewportFocused(false),
			m_ViewportHovered(false),
			m_ModifyFlags(0),
			m_MoveSpeed(100.0f),
			m_OldMousePosition(0.0f)
		{
			m_Texture = Texture2D::Create({}, "Assets/Textures/Gui/icons.png");
			float divisor = 4.0f;
			float width = (float)m_Texture->GetWidth();
			float height = (float)m_Texture->GetHeight();
			glm::vec2 size = glm::vec2(width / divisor, height / divisor);

			m_ButtonTexCoords[PlayButton] = CalculateTexCoords(glm::vec2(0, 2), size, { width, height });
			m_ButtonTexCoords[StopButton] = CalculateTexCoords(glm::vec2(3, 2), size, { width, height });

			uint32_t windowWidth = Application::Get().GetWindow().GetWidth();
			uint32_t windowHeight = Application::Get().GetWindow().GetHeight();
			m_EditorCamera.SetViewportSize((float)windowWidth, (float)windowHeight);
		}
		void ScenePanel::SetContext(const Ref<Scene>& context)
		{
			m_Context = context;
		}

		void ScenePanel::OnUpdate(Timestep ts)
		{			
			if (m_Context.Raw())
			{
				if (m_ViewportHovered && m_Context->GetState() == SceneState::Edit)
				{
					m_EditorCamera.OnUpdate(ts);
				}
				SceneEntity selected = m_Context->GetSelectedEntity();
				if (selected)
					showSelection(selected);
			}
		}	
		void ScenePanel::OnImGuiRender()
		{
			if (ImGui::Begin("Scene"))
			{
				if (m_Context.Raw())
				{				
					ImVec2 startCursorPos = ImGui::GetCursorPos();
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

					ImGui::Image(reinterpret_cast<void*>((void*)(uint64_t)SceneRenderer::GetFinalColorBufferRendererID()), ImVec2{ m_ViewportSize.x, m_ViewportSize.y }, ImVec2{ 0, 1 }, ImVec2{ 1, 0 });

					auto [mx, my] = getMouseViewportSpace();
					if (m_ComponentEditCallback)
					{
						m_ComponentEditCallback({ mx,my });
					}
					else if (m_ViewportHovered && m_ViewportFocused)
					{
						handleSelection({ mx,my });
					}

					ImGui::SetCursorPos(startCursorPos);
					if (m_Context->GetState() == SceneState::Edit)
					{
						ImGui::PushID("Play");
						if (ImGui::ImageButton((void*)(uint64_t)m_Texture->GetRendererID(),
							{ m_ButtonSize.x, m_ButtonSize.y },
							{ m_ButtonTexCoords[PlayButton].x, m_ButtonTexCoords[PlayButton].y },
							{ m_ButtonTexCoords[PlayButton].z, m_ButtonTexCoords[PlayButton].w }
						))
						{
							m_Context->SetState(SceneState::Play);
							m_Context->OnPlay();
						}
						ImGui::PopID();
					}
					else if (m_Context->GetState() == SceneState::Play)
					{
						ImGui::PushID("Stop");
						if (ImGui::ImageButton((void*)(uint64_t)m_Texture->GetRendererID(),
							{ m_ButtonSize.x, m_ButtonSize.y },
							{ m_ButtonTexCoords[StopButton].x, m_ButtonTexCoords[StopButton].y },
							{ m_ButtonTexCoords[StopButton].z, m_ButtonTexCoords[StopButton].w }
						))
						{
							m_Context->SetState(SceneState::Edit);
							m_Context->OnStop();
						}
						ImGui::PopID();
					}
				}
			}
			ImGui::End();
		}
	}
}