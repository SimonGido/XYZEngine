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

#include <imgui.h>
#include <imgui_internal.h>
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

			static AABB SceneEntityAABB(const SceneEntity& entity)
			{
				const TransformComponent& transformComponent = entity.GetComponent<TransformComponent>();
				auto [translation, rotation, scale] = transformComponent.GetWorldComponents();
				return AABB(
					translation - (scale / 2.0f),
					translation + (scale / 2.0f)
				);
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
			m_ModifyFlags(None),
			m_GizmoType(sc_InvalidGizmoType)
		{
			m_Texture = Texture2D::Create("Assets/Textures/Gui/icons.png");
			const float divisor = 4.0f;
			float width = (float)m_Texture->GetWidth();
			float height = (float)m_Texture->GetHeight();
			const glm::vec2 cellSize = glm::vec2(width / divisor, height / divisor);
			const glm::vec2 textureSize = { width, height };


			m_ButtonUVs[PlayButton] = UV::Calculate(glm::vec2(0, 2), cellSize, textureSize);
			m_ButtonUVs[StopButton] = UV::Calculate(glm::vec2(3, 2), cellSize, textureSize);

			const uint32_t windowWidth = Application::Get().GetWindow().GetWidth();
			const uint32_t windowHeight = Application::Get().GetWindow().GetHeight();
			
			m_EditorCamera.SetViewportSize((float)windowWidth, (float)windowHeight);
		}
	
		ScenePanel::~ScenePanel()
		{
		}

	
		void ScenePanel::OnImGuiRender(bool& open)
		{
			UI::ScopedStyleStack styleStack(ImGuiStyleVar_WindowPadding, ImVec2(0, 0));
			if (ImGui::Begin("Scene", &open, ImGuiWindowFlags_NoScrollbar | ImGuiWindowFlags_NoScrollWithMouse))
			{
				if (m_Context.Raw())
				{		
					const ImVec2 viewportPanelSize = ImGui::GetContentRegionAvail();
					m_ViewportBounds  = Utils::ImGuiViewportBounds();
					m_ViewportFocused = ImGui::IsWindowFocused();
					m_ViewportHovered = ImGui::IsWindowHovered();

					ImGuiLayer* imguiLayer = Application::Get().GetImGuiLayer();
					const bool blocked = imguiLayer->GetBlockedEvents();
					// Only unlock possible here
					imguiLayer->BlockEvents(blocked && !m_ViewportFocused && !m_ViewportHovered);				
									
					
					UI::Image(m_SceneRenderer->GetFinalPassImage(), viewportPanelSize);

					auto [mx, my] = getMouseViewportSpace();
					if (m_ViewportHovered && m_ViewportFocused && m_Context->GetState() == SceneState::Edit)
					{
						const SceneEntity selectedEntity = m_Context->GetSelectedEntity();
						if (selectedEntity && m_GizmoType != sc_InvalidGizmoType)
						{
							handleEntityTransform(m_Context->GetSelectedEntity());
						}
						else
						{
							handleSelection({ mx,my });
						}
					}
					handlePanelResize({ viewportPanelSize.x, viewportPanelSize.y });
					playBar();
					toolsBar();
				}
			}
			ImGui::End();		
		}

		void ScenePanel::OnUpdate(Timestep ts)
		{
			if (m_Context.Raw())
			{
				if (m_Context->GetState() == SceneState::Edit)
				{
					m_EditorCamera.OnUpdate(ts);			
					m_Context->OnRenderEditor(m_SceneRenderer, m_EditorCamera.GetViewProjection(), m_EditorCamera.GetViewMatrix(), m_EditorCamera.GetPosition(), ts);
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

		std::deque<SceneEntity> ScenePanel::findSelection(const Ray& ray)
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

		void ScenePanel::playBar()
		{
			const float toolbarWidth = 4.0f + 2.0f * m_ButtonSize.x;
			const float toolbarPositionX = (ImGui::GetWindowContentRegionWidth() - toolbarWidth) / 2.0f;
			UI::Toolbar(glm::vec2(toolbarPositionX, 8.0f), glm::vec2(4.0f, 0.0f), false,
				[&]() {
				UI::ScopedItemFlags flags(ImGuiItemFlags_Disabled, m_Context->GetState() == SceneState::Play);
				if (UI::ImageButtonTransparent("Play", m_Texture->GetImage(),
					m_ButtonSize,
					ImVec4(0.7f, 0.7f, 0.7f, 1.0f),
					ImVec4(1.0f, 1.0f, 1.0f, 1.0f),
					ImVec4(0.5f, 0.5f, 0.5f, 1.0f),
					m_ButtonUVs[PlayButton].UV0,
					m_ButtonUVs[PlayButton].UV1
				))
				{
					m_Context->SetState(SceneState::Play);
					m_Context->OnPlay();
				}
			},
				[&]() {
				UI::ScopedItemFlags flags(ImGuiItemFlags_Disabled, m_Context->GetState() == SceneState::Edit);
				if (UI::ImageButtonTransparent("Stop", m_Texture->GetImage(),
					m_ButtonSize,
					ImVec4(0.7f, 0.7f, 0.7f, 1.0f),
					ImVec4(1.0f, 1.0f, 1.0f, 1.0f),
					ImVec4(0.5f, 0.5f, 0.5f, 1.0f),
					m_ButtonUVs[StopButton].UV0,
					m_ButtonUVs[StopButton].UV1
				))
				{
					m_Context->SetState(SceneState::Edit);
					m_Context->OnStop();
				}
			});
		}

		void ScenePanel::toolsBar()
		{
			UI::Toolbar(glm::vec2(8.0f, 8.0f), glm::vec2(0.0f, 0.0f), false,
				[&]() {
					UI::ImageButton("##Tmp", m_Texture->GetImage(), m_ButtonSize, 0,
					ImVec4(0.7f, 0.7f, 0.7f, 1.0f), m_ButtonUVs[StopButton].UV0, m_ButtonUVs[StopButton].UV1);
				},
				[&]() {
					UI::ImageButton("##Tmp", m_Texture->GetImage(), m_ButtonSize, 0,
						ImVec4(0.7f, 0.7f, 0.7f, 1.0f), m_ButtonUVs[StopButton].UV0, m_ButtonUVs[StopButton].UV1);
				},
				[&]() {
					UI::ImageButton("##Tmp", m_Texture->GetImage(), m_ButtonSize, 0,
						ImVec4(0.7f, 0.7f, 0.7f, 1.0f), m_ButtonUVs[StopButton].UV0, m_ButtonUVs[StopButton].UV1);
				},
				[&]() {
					UI::ImageButton("##Tmp", m_Texture->GetImage(), m_ButtonSize, 0,
						ImVec4(0.7f, 0.7f, 0.7f, 1.0f), m_ButtonUVs[StopButton].UV0, m_ButtonUVs[StopButton].UV1);
				}
			);
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
				m_Context->SetSelectedEntity(Entity());

				std::deque<SceneEntity> newSelection = findSelection(ray);
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
					Application::Get().OnEvent(EntitySelectedEvent(m_Selection[m_SelectionIndex]));
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

		ScenePanel::UV ScenePanel::UV::Calculate(const glm::vec2& coords, const glm::vec2& cellSize, const glm::vec2& textureSize)
		{
			UV uv;
			uv.UV0 = { (coords.x * cellSize.x) / textureSize.x,
					 ((coords.y + 1) * cellSize.y) / textureSize.y };
			uv.UV1 = { ((coords.x + 1) * cellSize.x) / textureSize.x,
					   (coords.y * cellSize.y) / textureSize.y };

			return uv;
		}
	}
}