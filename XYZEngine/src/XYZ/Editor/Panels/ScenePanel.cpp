#include "stdafx.h"
#include "ScenePanel.h"

#include "XYZ/Core/Input.h"
#include "XYZ/Core/Application.h"

#include "XYZ/Renderer/SceneRenderer.h"
#include "XYZ/Renderer/Renderer2D.h"
#include "XYZ/Renderer/Renderer2D.h"
#include "XYZ/Utils/Math/Math.h"
#include "XYZ/ImGui/ImGui.h"

#include <imgui.h>
#include <ImGuizmo.h>

#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

namespace XYZ {
	namespace Editor {
		namespace Utils {
			template <typename T>
			static bool CompareDeques(const std::deque<T>& a, const std::deque<T>& b)
			{
				if (a.size() != b.size())
					return false;
				for (size_t i = 0; i < a.size(); ++i)
				{
					if (a[i] != b[i])
						return false;
				}
				return true;
			}

			static std::pair<glm::vec2, glm::vec2> CalculateTexCoords(const glm::vec2& coords, const glm::vec2& size, const glm::vec2& textureSize)
			{
				return {
					glm::vec2((coords.x * size.x) / textureSize.x, (coords.y * size.y) / textureSize.y),
					glm::vec2(((coords.x + 1) * size.x) / textureSize.x, ((coords.y + 1) * size.y) / textureSize.y)
				};
			}

			static AABB SceneEntityAABB(const SceneEntity& entity)
			{
				const TransformComponent& transformComponent = entity.GetComponent<TransformComponent>();
				auto [translation, rotation, scale] = transformComponent.GetWorldComponents();
				return AABB(
					translation - (scale / 2.0f),
					translation + (scale / 2.0f)
				);
			}
	
			static ImVec2 GlmToImVec2(const glm::vec2& val)
			{
				return ImVec2(val.x, val.y);
			}

			static std::array<glm::vec2, 2> ImGuiViewportBounds()
			{
				const auto viewportMinRegion = ImGui::GetWindowContentRegionMin();
				const auto viewportMaxRegion = ImGui::GetWindowContentRegionMax();
				const auto viewportOffset = ImGui::GetWindowPos();

				std::array<glm::vec2, 2> result;
				result[0] = { viewportMinRegion.x + viewportOffset.x, viewportMinRegion.y + viewportOffset.y };
				result[1] = { viewportMaxRegion.x + viewportOffset.x, viewportMaxRegion.y + viewportOffset.y };
				return result;
			}
		}
	
		ScenePanel::ScenePanel()
			:
			m_ViewportSize(0.0f),
			m_ButtonSize(15.0f),
			m_EditorCamera(30.0f, 1.778f, 0.1f, 1000.0f),
			m_ViewportFocused(false),
			m_ViewportHovered(false),
			m_SelectionIndex(0),
			m_ModifyFlags(0),
			m_MoveSpeed(100.0f),
			m_OldMousePosition(0.0f),
			m_GizmoType(-1)
		{
			m_Texture = Texture2D::Create("Assets/Textures/Gui/icons.png");
			const float divisor = 4.0f;
			float width = (float)m_Texture->GetWidth();
			float height = (float)m_Texture->GetHeight();
			const glm::vec2 size = glm::vec2(width / divisor, height / divisor);

			{
				auto [uv0, uv1] = Utils::CalculateTexCoords(glm::vec2(0, 1), size, { width, height });
				m_ButtonUVs[PlayButton].UV0 = uv0;
				m_ButtonUVs[PlayButton].UV1 = uv1;
			}
			{
				auto [uv0, uv1] = Utils::CalculateTexCoords(glm::vec2(3, 1), size, { width, height });
				m_ButtonUVs[StopButton].UV0 = uv0;
				m_ButtonUVs[StopButton].UV1 = uv1;
			}

			const uint32_t windowWidth = Application::Get().GetWindow().GetWidth();
			const uint32_t windowHeight = Application::Get().GetWindow().GetHeight();
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
		void ScenePanel::OnImGuiRender(const Ref<Image2D>& finalImage)
		{
			if (ImGui::Begin("Scene", nullptr, ImGuiWindowFlags_MenuBar))
			{
				if (m_Context.Raw())
				{
					const ImVec2 startCursorPos = ImGui::GetCursorPos();
					m_ViewportBounds = Utils::ImGuiViewportBounds();
					m_ViewportFocused = ImGui::IsWindowFocused();
					m_ViewportHovered = ImGui::IsWindowHovered();

					ImGuiLayer* imguiLayer = Application::Get().GetImGuiLayer();
					const bool blocked = imguiLayer->GetBlockedEvents();
					if (blocked)
						imguiLayer->BlockEvents(!m_ViewportFocused && !m_ViewportHovered);


					ImVec2 viewportPanelSize = ImGui::GetContentRegionAvail();
					handlePanelResize({ viewportPanelSize.x, viewportPanelSize.y });

					UI::Image(finalImage, viewportPanelSize);
					
					auto [mx, my] = getMouseViewportSpace();
					if (m_ViewportHovered && m_ViewportFocused && m_Context->GetState() == SceneState::Edit)
					{
						const SceneEntity selectedEntity = m_Context->GetSelectedEntity();
						if (selectedEntity && m_GizmoType != -1)
						{
							handleEntityTransform(m_Context->GetSelectedEntity());
						}
						else
						{
							handleSelection({ mx,my });
						}
					}
					if (ImGui::BeginMenuBar())
					{
						
						//ImGui::SetCursorPos(startCursorPos);
						if (m_Context->GetState() == SceneState::Edit)
						{
							if (UI::ImageButton("Play", m_Texture->GetImage(),
								Utils::GlmToImVec2(m_ButtonSize),
								ImVec4(0.7f, 0.7f, 0.7f, 1.0f),
								ImVec4(1.0f, 1.0f, 1.0f, 1.0f),
								ImVec4(0.5f, 0.5f, 0.5f, 1.0f),
								Utils::GlmToImVec2(m_ButtonUVs[PlayButton].UV0),
								Utils::GlmToImVec2(m_ButtonUVs[PlayButton].UV1)
							))
							{
								m_Context->SetState(SceneState::Play);
								m_Context->OnPlay();
							}
						}
						else if (m_Context->GetState() == SceneState::Play)
						{
							if (UI::ImageButton("Stop", m_Texture->GetImage(),
								Utils::GlmToImVec2(m_ButtonSize),
								ImVec4(0.7f, 0.7f, 0.7f, 1.0f),
								ImVec4(1.0f, 1.0f, 1.0f, 1.0f),
								ImVec4(0.5f, 0.5f, 0.5f, 1.0f),
								Utils::GlmToImVec2(m_ButtonUVs[StopButton].UV0),
								Utils::GlmToImVec2(m_ButtonUVs[StopButton].UV1)
							))
							{
								m_Context->SetState(SceneState::Edit);
								m_Context->OnStop();
							}
						}
						ImGui::EndMenuBar();
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
		std::pair<glm::vec3, glm::vec3> ScenePanel::castRay(float mx, float my) const
		{
			const glm::vec4 mouseClipPos = { mx, my, -1.0f, 1.0f };

			const auto inverseProj = glm::inverse(m_EditorCamera.GetProjectionMatrix());
			const auto inverseView = glm::inverse(glm::mat3(m_EditorCamera.GetViewMatrix()));

			const glm::vec4 ray = inverseProj * mouseClipPos;
			glm::vec3 rayPos = m_EditorCamera.GetPosition();
			glm::vec3 rayDir = inverseView * glm::vec3(ray);

			return { rayPos, rayDir };
		}

		std::pair<float, float> ScenePanel::getMouseViewportSpace() const
		{
			auto [mx, my] = Input::GetMousePosition();
			auto [winPosX, winPosY] = Input::GetWindowPosition();
			mx -= ImGui::GetWindowPos().x;
			my -= ImGui::GetWindowPos().y;
			mx += winPosX;
			my += winPosY;

			const auto viewportWidth = ImGui::GetWindowSize().x;
			const auto viewportHeight = ImGui::GetWindowSize().y;

			return { (mx / viewportWidth) * 2.0f - 1.0f, ((my / viewportHeight) * 2.0f - 1.0f) * -1.0f };
		}

		std::deque<SceneEntity> ScenePanel::getSelection(const Ray& ray)
		{
			std::deque<SceneEntity> result;
			for (const Entity entityID : m_Context->GetEntities())
			{
				SceneEntity entity(entityID, m_Context.Raw());
				m_Context->GetECS();
				if (ray.IntersectsAABB(Utils::SceneEntityAABB(entity)))
				{
					result.push_back(entity);
				}
			}
			std::sort(result.begin(), result.end(), [&](const SceneEntity& a, const SceneEntity& b) {
				auto& cameraPos = m_EditorCamera.GetPosition();
				const TransformComponent& transformA = a.GetComponent<TransformComponent>();
				const TransformComponent& transformB = b.GetComponent<TransformComponent>();
				auto [worldPosA, worldRotA, worldScaleA] = transformA.GetWorldComponents();
				auto [worldPosB, worldRotB, worldScaleB] = transformB.GetWorldComponents();
				return glm::distance(worldPosA, cameraPos) < glm::distance(worldPosB, cameraPos);
			});
			return result;
		}


		void ScenePanel::handlePanelResize(const glm::vec2& newSize)
		{
			if (m_ViewportSize.x != newSize.x || m_ViewportSize.y != newSize.y)
			{
				m_ViewportSize = newSize;
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
				const Ray ray = { origin,direction };
				m_Context->SetSelectedEntity(Entity());

				std::deque<SceneEntity> newSelection = getSelection(ray);
				if (!Utils::CompareDeques(m_Selection, newSelection))
				{
					m_Selection = std::move(newSelection);
					m_SelectionIndex = 0;
				}
				else if (static_cast<size_t>(m_SelectionIndex) + 1 == m_Selection.size())
				{
					m_SelectionIndex = 0;
				}
				else
				{
					m_SelectionIndex++;
				}

				if (!m_Selection.empty())
				{
					m_Context->SetSelectedEntity(m_Selection[m_SelectionIndex]);
				}
			}
		}

		void ScenePanel::handleEntityTransform(SceneEntity entity)
		{
			ImGuizmo::SetOrthographic(false);
			ImGuizmo::SetDrawlist();

			ImGuizmo::SetRect(m_ViewportBounds[0].x, m_ViewportBounds[0].y, m_ViewportBounds[1].x - m_ViewportBounds[0].x, m_ViewportBounds[1].y - m_ViewportBounds[0].y);
			const glm::mat4& cameraProjection = m_EditorCamera.GetProjectionMatrix();
			glm::mat4 cameraView = m_EditorCamera.GetViewMatrix();

			const ECSManager& ecs = m_Context->GetECS();

			SceneEntity selectedEntity = m_Context->GetSelectedEntity();
			TransformComponent& tc = selectedEntity.GetComponent<TransformComponent>();
			const Relationship& rel = selectedEntity.GetComponent<Relationship>();

			glm::mat4 transform = tc.GetTransform();
			// This is hack to translate the gizmo to the proper position based on hierarchy transformation
			if (rel.GetParent())
			{
				cameraView *= ecs.GetComponent<TransformComponent>(rel.GetParent()).WorldTransform;
			}
			ImGuizmo::Manipulate(glm::value_ptr(cameraView), glm::value_ptr(cameraProjection),
				(ImGuizmo::OPERATION)m_GizmoType, ImGuizmo::LOCAL, glm::value_ptr(transform),
				nullptr, nullptr);

			if (ImGuizmo::IsUsing())
			{
				tc.DecomposeTransform(transform);
			}
		}
	}
}