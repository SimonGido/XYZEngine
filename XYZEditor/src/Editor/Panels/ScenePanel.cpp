#include "stdafx.h"
#include "ScenePanel.h"

#include "XYZ/Core/Input.h"
#include "XYZ/Core/Application.h"

#include "XYZ/Renderer/SceneRenderer.h"
#include "XYZ/Renderer/Renderer2D.h"
#include "XYZ/Renderer/Renderer2D.h"


#include "XYZ/Utils/Math/Math.h"
#include "XYZ/ImGui/ImGui.h"

#include "Editor/Event/EditorEvents.h"
#include "Editor/EditorHelper.h"
#include "EditorLayer.h"

#include <imgui.h>
#include <imgui_internal.h>
#include <ImGuizmo.h>

#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

namespace XYZ {
	namespace Editor {
		namespace Utils {
			template <typename T, typename Comparator>
			static bool CompareDeques(const std::deque<T>& a, const std::deque<T>& b, Comparator&& cmp)
			{
				if (a.size() != b.size())
					return false;
				for (size_t i = 0; i < a.size(); ++i)
				{
					if (!cmp(a[i], b[i]))
						return false;
				}
				return true;
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

		ScenePanel::ScenePanel(std::string name)
			:
			EditorPanel(std::move(name)),
			m_ViewportSize(0.0f),
			m_ButtonSize(25.0f),
			m_EditorCamera(30.0f, 1.778f, 0.1f, 1000.0f),
			m_ViewportFocused(false),
			m_ViewportHovered(false),
			m_SelectionIndex(0),
			m_GizmoType(sc_InvalidGizmoType)
		{
			const uint32_t windowWidth = Application::Get().GetWindow().GetWidth();
			const uint32_t windowHeight = Application::Get().GetWindow().GetHeight();

			m_EditorCamera.SetViewportSize((float)windowWidth, (float)windowHeight);
		}

		ScenePanel::~ScenePanel()
		{
		}


		void ScenePanel::OnImGuiRender(bool& open)
		{
			{
				UI::ScopedStyleStack styleStack(true, ImGuiStyleVar_WindowPadding, ImVec2(0, 0));
				if (ImGui::Begin("Scene", &open, ImGuiWindowFlags_NoScrollbar | ImGuiWindowFlags_NoScrollWithMouse))
				{
					if (m_Context.Raw())
					{
						const ImVec2 viewportPanelSize = ImGui::GetContentRegionAvail();
						m_ViewportBounds = Utils::ImGuiViewportBounds();
						m_ViewportFocused = ImGui::IsWindowFocused();
						m_ViewportHovered = ImGui::IsWindowHovered();

						ImGuiLayer* imguiLayer = Application::Get().GetImGuiLayer();
						const bool blocked = imguiLayer->GetBlockedEvents();
						// Only unlock possible here
						imguiLayer->BlockEvents(blocked && !m_ViewportFocused && !m_ViewportHovered);


						UI::Image(m_SceneRenderer->GetFinalPassImage(), viewportPanelSize);
						if (m_Context->GetState() == SceneState::Edit)
							acceptDragAndDrop();

						bool handled = playBar();
						handled |= toolsBar();

						if (m_ViewportHovered && m_ViewportFocused && m_Context->GetState() == SceneState::Edit)
						{
							const SceneEntity selectedEntity = m_Context->GetSelectedEntity();
							if (selectedEntity && m_GizmoType != sc_InvalidGizmoType)
							{
								handleEntityTransform(m_Context->GetSelectedEntity());
							}
							else if (!handled)
							{
								auto [mx, my] = getMouseViewportSpace();
								handleSelection({ mx,my });
							}
						}
						handlePanelResize({ viewportPanelSize.x, viewportPanelSize.y });
					}
				}
				ImGui::End();
			}
			if (m_Context.Raw())
			{
				m_Context->OnImGuiRender();
			}
		}

		void ScenePanel::OnUpdate(Timestep ts)
		{
			if (m_Context.Raw())
			{
				if (m_Context->GetState() == SceneState::Edit)
				{
					if (m_ViewportHovered && m_ViewportFocused)
						m_EditorCamera.OnUpdate(ts);
					m_Context->OnUpdateEditor(ts);
					m_Context->OnRenderEditor(m_SceneRenderer, m_EditorCamera.GetViewProjection(), m_EditorCamera.GetViewMatrix(), m_EditorCamera.GetPosition());
				}
				else
				{
					m_Context->OnUpdate(ts);
					m_Context->OnRender(m_SceneRenderer);
				}
			}
		}

		bool ScenePanel::OnEvent(Event& event)
		{
			EventDispatcher dispatcher(event);
			dispatcher.Dispatch<KeyPressedEvent>(Hook(&ScenePanel::onKeyPressed, this));
			if (m_ViewportHovered && m_ViewportFocused)
				m_EditorCamera.OnEvent(event);

			return false;
		}

		void ScenePanel::SetSceneContext(const Ref<Scene>& context)
		{
			m_Context = context;
			if (m_SceneRenderer.Raw())
			{
				m_SceneRenderer->SetScene(m_Context);
			}
		}

		void ScenePanel::SetSceneRenderer(const Ref<SceneRenderer>& sceneRenderer)
		{
			m_SceneRenderer = sceneRenderer;
			if (m_Context.Raw())
			{
				m_SceneRenderer->SetScene(m_Context);
			}
		}

		bool ScenePanel::onKeyPressed(KeyPressedEvent& e)
		{
			if (m_ViewportFocused && m_ViewportHovered)
			{
				if (e.IsKeyPressed(KeyCode::KEY_Q))
				{
					if (!ImGuizmo::IsUsing())
						m_GizmoType = sc_InvalidGizmoType;
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

		bool ScenePanel::playBar()
		{
			bool handled = false;
			const float toolbarWidth = 4.0f + 2.0f * m_ButtonSize.x;
			const float toolbarPositionX = (ImGui::GetWindowContentRegionWidth() - toolbarWidth) / 2.0f;
			
			UI::Toolbar(glm::vec2(toolbarPositionX, 8.0f), glm::vec2(4.0f, 0.0f), false,
				[&]() {
				UI::ScopedItemFlags flags(ImGuiItemFlags_Disabled, m_Context->GetState() == SceneState::Play);
				if (ButtonTransparent("Play", m_ButtonSize, ED::ArrowIcon))
				{
					m_Context->SetState(SceneState::Play);
					m_Context->OnPlay();
					handled = true;
				}
			},
				[&]() {
				UI::ScopedItemFlags flags(ImGuiItemFlags_Disabled, m_Context->GetState() == SceneState::Edit);
				if (ButtonTransparent("Stop", m_ButtonSize, ED::StopIcon))
				{
					m_Context->SetState(SceneState::Edit);
					m_Context->OnStop();
					handled = true;
				}
			});
			return handled;
		}

		bool ScenePanel::toolsBar()
		{
			bool handled = false;
	
			UI::Toolbar(glm::vec2(8.0f, 8.0f), glm::vec2(0.0f, 0.0f), false,
				[&]() {
					if (ButtonTransparent("##Cursor", m_ButtonSize, ED::CursorIcon))
					{
						m_GizmoType = sc_InvalidGizmoType;
						handled = true;
					}
				},
				[&]() {
					if (ButtonTransparent("##Move", m_ButtonSize, ED::MoveIcon))
					{
						m_GizmoType = ImGuizmo::OPERATION::TRANSLATE;
						handled = true;
					}
				},
				[&]() {
					if (ButtonTransparent("##Rotate", m_ButtonSize, ED::RotateIcon))
					{
						m_GizmoType = ImGuizmo::OPERATION::ROTATE;
						handled = true;
					}
				},
				[&]() {
					if (ButtonTransparent("##Scale", m_ButtonSize, ED::ScaleIcon))
					{
						m_GizmoType = ImGuizmo::OPERATION::SCALE;
						handled = true;
					}
				}
			);
			return handled;
		}

		void ScenePanel::handlePanelResize(const glm::vec2& newSize)
		{
			if (m_ViewportSize.x != newSize.x || m_ViewportSize.y != newSize.y)
			{
				m_ViewportSize = newSize;
				m_EditorCamera.SetViewportSize(m_ViewportSize.x, m_ViewportSize.y);
				m_Context->SetViewportSize(static_cast<uint32_t>(m_ViewportSize.x), static_cast<uint32_t>(m_ViewportSize.y));
				m_SceneRenderer->SetViewportSize(static_cast<uint32_t>(m_ViewportSize.x), static_cast<uint32_t>(m_ViewportSize.y));
			}
		}

		void ScenePanel::handleSelection(const glm::vec2& mousePosition)
		{
			if (ImGui::GetIO().MouseClicked[ImGuiMouseButton_Left]
				&& !ImGui::GetIO().KeyCtrl)
			{
				auto [origin, direction] = castRay(mousePosition.x, mousePosition.y);
				const Ray ray = { origin,direction };
				m_Context->SetSelectedEntity(entt::null);

				std::deque<SceneIntersection::HitData> newSelection = SceneIntersection::Intersect(ray, m_Context);
				
				if (!Utils::CompareDeques(m_Selection, newSelection, [](const SceneIntersection::HitData& a, const SceneIntersection::HitData & b) {
					return a.Entity == b.Entity;
				}))
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
					m_Context->SetSelectedEntity(m_Selection[m_SelectionIndex].Entity.ID());
					Application::Get().OnEvent(EntitySelectedEvent(m_Selection[m_SelectionIndex].Entity));
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

			

			SceneEntity selectedEntity = m_Context->GetSelectedEntity();
			TransformComponent& tc = selectedEntity.GetComponent<TransformComponent>();
			const Relationship& rel = selectedEntity.GetComponent<Relationship>();

			glm::mat4 transform = tc->WorldTransform;
			
			ImGuizmo::Manipulate(glm::value_ptr(cameraView), glm::value_ptr(cameraProjection),
				(ImGuizmo::OPERATION)m_GizmoType, ImGuizmo::LOCAL, glm::value_ptr(transform),
				nullptr, nullptr);

			if (ImGuizmo::IsUsing())
			{
				auto parent = rel.GetParent();
				if (rel.GetParent() != entt::null)
				{
					auto& reg = *entity.GetRegistry();
					const glm::mat4& parentTransform = reg.get<TransformComponent>(parent)->WorldTransform;
					transform = glm::inverse(parentTransform) * transform;
				}
				auto [translation, rotation, scale] = Math::DecomposeTransform(transform);
				glm::vec3 deltaRot = rotation - tc->Rotation;
				tc.GetTransform().Translation = translation;
				tc.GetTransform().Rotation += deltaRot;
				tc.GetTransform().Scale = scale;
			}
		}

		void ScenePanel::acceptDragAndDrop()
		{
			char* assetPath = nullptr;
			if (UI::DragDropTarget("AssetDragAndDrop", &assetPath))
			{
				std::filesystem::path path(assetPath);
				if (AssetManager::Exist(path))
				{
					const auto& metadata = AssetManager::GetMetadata(path);
					if (metadata.Type == AssetType::Prefab)
					{
						Ref<Prefab> prefab = AssetManager::GetAsset<Prefab>(metadata.Handle);

						auto [mx, my] = getMouseViewportSpace();
						auto [origin, direction] = castRay(mx, my);
						const Ray ray = { origin,direction };
						const float boundary = 9999.9f;

						const AABB aabb{ glm::vec3(-boundary, -boundary, 0.0f), glm::vec3(boundary, boundary, 0.0f) };
						float distance = 0.0f;
						ray.IntersectsAABB(aabb, distance);

						glm::vec3 translation = ray.Origin + ray.Direction * distance;

						prefab->Instantiate(m_Context, SceneEntity(), &translation);
					}
					else if (metadata.Type == AssetType::Scene)
					{
						Ref<Scene> scene = AssetManager::GetAsset<Scene>(metadata.Handle);
						scene->SetViewportSize(static_cast<uint32_t>(m_ViewportSize.x), static_cast<uint32_t>(m_ViewportSize.y));
						Application::Get().OnEvent(SceneLoadedEvent(scene));
					}
				}
			}
		}
	}
}