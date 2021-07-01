#include "stdafx.h"
#include "ScenePanel.h"

#include "XYZ/Core/Input.h"
#include "XYZ/Core/Application.h"
#include "XYZ/Scene/SceneEntity.h"
#include "XYZ/Renderer/SceneRenderer.h"
#include "XYZ/Renderer/Renderer2D.h"
#include "XYZ/Utils/Math/Ray.h"
#include "XYZ/Utils/Math/Math.h"

#include <imgui.h>
#include <ImGuizmo.h>

#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

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
		}

		static AABB SceneEntityAABB(SceneEntity entity)
		{
			TransformComponent& transformComponent = entity.GetComponent<TransformComponent>();
			auto [translation, rotation, scale] = transformComponent.GetWorldComponents();
			return AABB(
				translation - (scale / 2.0f),
				translation + (scale / 2.0f)
			);
		}

		void ScenePanel::handlePanelResize(const glm::vec2& newSize)
		{
			if (m_ViewportSize.x != newSize.x || m_ViewportSize.y != newSize.y)
			{
				m_ViewportSize = newSize;
				SceneRenderer::SetViewportSize((uint32_t)m_ViewportSize.x, (uint32_t)m_ViewportSize.y);
				m_EditorCamera.SetViewportSize(m_ViewportSize.x, m_ViewportSize.y);
				m_Context->SetViewportSize((uint32_t)m_ViewportSize.x, (uint32_t)m_ViewportSize.y);
			}
		}

		void ScenePanel::handleSelection(const glm::vec2& mousePosition)
		{
			if (ImGui::GetIO().MouseClicked[ImGuiMouseButton_Left]
			&& !ImGui::GetIO().KeyCtrl)
			{
				auto [origin, direction] = castRay(mousePosition.x, mousePosition.y);
				Ray ray = { origin,direction };
				m_Context->SetSelectedEntity(Entity());
				if (m_Callback)
					m_Callback(m_Context->GetSelectedEntity());
				
				// First check old selection
				while (!m_Selection.empty())
				{
					Entity first = m_Selection.front();
					m_Selection.pop_front();
					SceneEntity entity(first, m_Context.Raw());		
					if (ray.IntersectsAABB(SceneEntityAABB(entity)))
					{
						m_Context->SetSelectedEntity(first);
						if (m_Callback)
							m_Callback(m_Context->GetSelectedEntity());
						return;
					}
				}

				m_Selection.clear();
				bool selected = false;
				for (Entity entityID : m_Context->GetEntities())
				{
					SceneEntity entity(entityID, m_Context.Raw());
					if (ray.IntersectsAABB(SceneEntityAABB(entity)))
					{
						// Do not add first selected to the selection deque
						if (selected)
						{
							m_Selection.push_back(entityID);
						}
						else
						{
							selected = true;
							m_Context->SetSelectedEntity(entityID);
							if (m_Callback)
								m_Callback(m_Context->GetSelectedEntity());
						}					
					}
				}
			}
		}

		void ScenePanel::handleEntityTransform(SceneEntity entity)
		{
			ImGuizmo::SetOrthographic(false);
			ImGuizmo::SetDrawlist();

			ImGuizmo::SetRect(m_ViewportBounds[0].x, m_ViewportBounds[0].y, m_ViewportBounds[1].x - m_ViewportBounds[0].x, m_ViewportBounds[1].y - m_ViewportBounds[0].y);
			const glm::mat4& cameraProjection = m_EditorCamera.GetProjectionMatrix();
			const glm::mat4& cameraView = m_EditorCamera.GetViewMatrix();

			auto& tc = m_Context->GetSelectedEntity().GetComponent<TransformComponent>();
			glm::mat4 transform = tc.GetTransform();

			ImGuizmo::Manipulate(glm::value_ptr(cameraView), glm::value_ptr(cameraProjection),
				(ImGuizmo::OPERATION)m_GizmoType, ImGuizmo::LOCAL, glm::value_ptr(transform),
				nullptr, nullptr);
			if (ImGuizmo::IsUsing())
			{
				tc.DecomposeTransform(transform);
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
			m_OldMousePosition(0.0f),
			m_GizmoType(-1)
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
		ScenePanel::~ScenePanel()
		{
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
					
					ImGuiLayer* imguiLayer = Application::Get().GetImGuiLayer();
					bool blocked = imguiLayer->GetBlockedEvents();
					if (blocked)
						imguiLayer->BlockEvents(!m_ViewportFocused && !m_ViewportHovered);

					
					ImVec2 viewportPanelSize = ImGui::GetContentRegionAvail();
					handlePanelResize({ viewportPanelSize.x, viewportPanelSize.y });

					ImGui::Image(reinterpret_cast<void*>((void*)(uint64_t)SceneRenderer::GetFinalColorBufferRendererID()), ImVec2{ m_ViewportSize.x, m_ViewportSize.y }, ImVec2{ 0, 1 }, ImVec2{ 1, 0 });

					auto [mx, my] = getMouseViewportSpace();
					if (m_ViewportHovered && m_ViewportFocused)
					{
						SceneEntity selectedEntity = m_Context->GetSelectedEntity();
						if (selectedEntity && m_GizmoType != -1)
						{
							handleEntityTransform(m_Context->GetSelectedEntity());
						}
						else
						{
							handleSelection({ mx,my });
						}
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
		void ScenePanel::OnEvent(Event& event)
		{
			EventDispatcher dispatcher(event);
			dispatcher.Dispatch<KeyPressedEvent>(Hook(&ScenePanel::onKeyPressed, this));
			if (m_ViewportHovered && m_ViewportFocused)
				m_EditorCamera.OnEvent(event);
		}

		bool ScenePanel::onKeyPressed(KeyPressedEvent& e)
		{
			if (m_ViewportFocused && m_ViewportHovered)
			{
				if (e.IsKeyPressed(KeyCode::KEY_Q))
				{
					if (!ImGuizmo::IsUsing())
						m_GizmoType = -1;
					return true;
				}
				else if (e.IsKeyPressed(KeyCode::KEY_W))
				{
					if (!ImGuizmo::IsUsing())
						m_GizmoType = ImGuizmo::OPERATION::TRANSLATE;
					return true;
				}
				else if (e.IsKeyPressed(KeyCode::KEY_E))
				{
					if (!ImGuizmo::IsUsing())
						m_GizmoType = ImGuizmo::OPERATION::ROTATE;
					return true;
				}
				else if (e.IsKeyPressed(KeyCode::KEY_R))
				{
					if (!ImGuizmo::IsUsing())
						m_GizmoType = ImGuizmo::OPERATION::SCALE;
					return true;
				}
			}
			return false;
		}
	}
}