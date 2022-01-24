#include "EditorLayer.h"

#include <glm/glm.hpp>
#include <glm/gtx/transform.hpp>

#include <random>

namespace XYZ {
	namespace Editor {

		EditorLayer::EditorLayer()
		{
			s_Data.Init();
		}

		EditorLayer::~EditorLayer()
		{
			s_Data.Shutdown();
		}

		void EditorLayer::OnAttach()
		{
			s_Data.Init();
			m_Scene = Ref<Scene>::Create("TestScene");
			m_Scene->SetViewportSize(
				Application::Get().GetWindow().GetWidth(),
				Application::Get().GetWindow().GetHeight()
			);
			ScriptEngine::Init("Assets/Scripts/XYZScript.dll");
			ScriptEngine::SetSceneContext(m_Scene);


			m_SceneRenderer = Ref<SceneRenderer>::Create(m_Scene, SceneRendererSpecification());
			m_CameraTexture = Texture2D::Create("Resources/Editor/Camera.png");
			m_CommandBuffer = RenderCommandBuffer::Create(0, "Overlay");
			m_CommandBuffer->CreateTimestampQueries(GPUTimeQueries::Count());
			m_OverlayRenderer2D = Ref<Renderer2D>::Create(m_CommandBuffer);
			m_QuadMaterial = Material::Create(Shader::Create("Resources/Shaders/DefaultShader.glsl"));
			m_OverlayRenderer2D->SetQuadMaterial(m_QuadMaterial);
			m_LineMaterial = Material::Create(Shader::Create("Resources/Shaders/LineShader.glsl"));
			m_OverlayRenderer2D->SetLineMaterial(m_LineMaterial);
			m_CircleMaterial = Material::Create(Shader::Create("Resources/Shaders/Circle.glsl"));
			m_OverlayRenderer2D->SetCircleMaterial(m_CircleMaterial);

			m_EditorManager.SetSceneContext(m_Scene);
			m_EditorManager.RegisterPanel<Editor::ScenePanel>("ScenePanel");
			m_EditorManager.RegisterPanel<Editor::InspectorPanel>("InspectorPanel");
			m_EditorManager.RegisterPanel<Editor::SceneHierarchyPanel>("SceneHierarchyPanel");
			m_EditorManager.RegisterPanel<Editor::ImGuiStylePanel>("ImGuiStylePanel");
			m_EditorManager.RegisterPanel<Editor::AssetManagerViewPanel>("AssetManagerViewPanel");
			m_EditorManager.RegisterPanel<Editor::AssetBrowser>("AssetBrowser");
			m_EditorManager.RegisterPanel<Editor::ScriptPanel>("ScriptPanel");

			Ref<Editor::ScenePanel> scenePanel = m_EditorManager.GetPanel<Editor::ScenePanel>("ScenePanel");
			scenePanel->SetSceneRenderer(m_SceneRenderer);
			m_EditorCamera = &scenePanel->GetEditorCamera();

			Renderer::WaitAndRenderAll();
			//AssetManager::CreateAsset<Texture2D>("Checkerboard.tex", "Assets/Textures", "Assets/Textures/checkerboard.png");
		}

		void EditorLayer::OnDetach()
		{
			SceneSerializer serializer(m_Scene);
			serializer.Serialize("Assets/Scenes/Scene.xyz");
			ScriptEngine::Shutdown();
			m_EditorManager.Clear();
		}
		void EditorLayer::OnUpdate(Timestep ts)
		{
			m_EditorManager.OnUpdate(ts);
			if (m_Scene->GetState() == SceneState::Edit)
				renderOverlay();

			if (m_SelectedEntity != m_Scene->GetSelectedEntity())
			{
				m_SelectedEntity = m_Scene->GetSelectedEntity();
			}
		}

		void EditorLayer::OnEvent(Event& event)
		{
			m_EditorManager.OnEvent(event);
		}

		void EditorLayer::OnImGuiRender()
		{
			m_SceneRenderer->OnImGuiRender();
			m_EditorManager.OnImGuiRender();
		}

		bool EditorLayer::onMouseButtonPress(MouseButtonPressEvent& event)
		{
			return false;
		}
		bool EditorLayer::onMouseButtonRelease(MouseButtonReleaseEvent& event)
		{
			return false;
		}
		bool EditorLayer::onWindowResize(WindowResizeEvent& event)
		{
			return false;
		}

		bool EditorLayer::onKeyPress(KeyPressedEvent& event)
		{
			return false;
		}

		bool EditorLayer::onKeyRelease(KeyReleasedEvent& event)
		{
			return false;
		}

		void EditorLayer::renderOverlay()
		{
			m_CommandBuffer->Begin();
			m_GPUTimeQueries.GPUTime = m_CommandBuffer->BeginTimestampQuery();
			m_OverlayRenderer2D->BeginScene(m_EditorCamera->GetViewProjection(), m_EditorCamera->GetViewMatrix());
			m_OverlayRenderer2D->SetTargetRenderPass(m_SceneRenderer->GetFinalRenderPass());

			renderSelected();
			renderColliders();
			renderCameras();
			renderLights();

			m_OverlayRenderer2D->EndScene(false);
			m_CommandBuffer->EndTimestampQuery(m_GPUTimeQueries.GPUTime);

			m_CommandBuffer->End();
			m_CommandBuffer->Submit();
			Application::Get().GetPerformanceProfiler().PushMeasurement("EditorLayer::renderOverlay", static_cast<float>(m_GPUTimeQueries.GPUTime));
		}

		void EditorLayer::renderColliders()
		{
			if (m_ShowColliders)
			{
				auto& ecs = m_Scene->GetECS();
				auto box2DColliderView = ecs.CreateView<TransformComponent, BoxCollider2DComponent>();
				for (auto entity : box2DColliderView)
				{
					auto [transformComp, boxCollider] = box2DColliderView.Get(entity);
					auto [translation, rotation, scale] = transformComp.GetWorldComponents();
					translation += glm::vec3(boxCollider.Offset, 0.0f);
					scale *= glm::vec3(boxCollider.Size, 1.0f);

					glm::mat4 transform = glm::translate(glm::mat4(1.0f), translation)
						* glm::rotate(glm::mat4(1.0f), rotation.z, glm::vec3(0.0f, 0.0f, 1.0f))
						* glm::scale(glm::mat4(1.0f), scale);

					m_OverlayRenderer2D->SubmitRect(transform, s_Data.Color[ED::Collider2D]);
				}

				auto circle2DColliderView = ecs.CreateView<TransformComponent, CircleCollider2DComponent>();
				for (auto entity : circle2DColliderView)
				{
					auto [transformComp, circleCollider] = circle2DColliderView.Get(entity);
					auto [translation, rotation, scale] = transformComp.GetWorldComponents();
					translation += glm::vec3(circleCollider.Offset, 0.0f);
					scale *= glm::vec3(circleCollider.Radius * 2.0f);
					m_OverlayRenderer2D->SubmitFilledCircle(translation, glm::vec2(scale.x, scale.y), 0.01f, s_Data.Color[ED::Collider2D]);
				}
			}
		}

		void EditorLayer::renderCameras()
		{
			if (m_ShowCameras)
			{
				auto& ecs = m_Scene->GetECS();

				auto cameraView = ecs.CreateView<TransformComponent, CameraComponent>();
				for (auto entity : cameraView)
				{
					auto [transform, camera] = cameraView.Get(entity);
					auto [min, max] = cameraToAABB(transform, camera.Camera);
					auto [translation, rotation, scale] = transform.GetWorldComponents();
					m_OverlayRenderer2D->SubmitQuadBillboard(translation, glm::vec2(scale.x, scale.y), glm::vec4(0.0f, 0.0f, 1.0f, 1.0f), m_CameraTexture);
				}
			}
		}

		void EditorLayer::renderLights()
		{
			if (m_ShowLights)
			{
				auto& ecs = m_Scene->GetECS();

				auto spotLight2DView = ecs.CreateView<TransformComponent, SpotLight2D>();


				auto pointLight2DView = ecs.CreateView<TransformComponent, PointLight2D>();
			}
		}

		void EditorLayer::renderSelected()
		{
			const SceneEntity selected = m_Scene->GetSelectedEntity();
			if (selected && selected != m_Scene->GetSceneEntity())
			{
				if (selected.HasComponent<CameraComponent>())
				{
					auto& transform = selected.GetComponent<TransformComponent>();
					auto& camera = selected.GetComponent<CameraComponent>();
					auto [min, max] = cameraToAABB(transform, camera.Camera);
					m_OverlayRenderer2D->SubmitAABB(min, max, s_Data.Color[ED::BoundingBox]);
				}
				else
				{
					auto& transform = selected.GetComponent<TransformComponent>();
					auto [min, max] = transformToAABB(transform);
					m_OverlayRenderer2D->SubmitAABB(min, max, s_Data.Color[ED::BoundingBox]);
				}
			}
		}

		std::pair<glm::vec3, glm::vec3> EditorLayer::cameraToAABB(const TransformComponent& transform, const SceneCamera& camera) const
		{
			auto [translation, rotation, scale] = transform.GetWorldComponents();
			if (camera.GetProjectionType() == CameraProjectionType::Orthographic)
			{
				auto& orthoProps = camera.GetOrthographicProperties();
				float size = orthoProps.OrthographicSize;
				float aspect = (float)camera.GetViewportWidth() / (float)camera.GetViewportHeight();
				float width = size * aspect;
				float height = size;

				glm::vec3 min = { translation.x - width / 2.0f,translation.y - height / 2.0f, translation.z + orthoProps.OrthographicNear };
				glm::vec3 max = { translation.x + width / 2.0f,translation.y + height / 2.0f, translation.z + orthoProps.OrthographicFar };
				return { min, max };
			}
			else
			{

			}
			return std::pair<glm::vec3, glm::vec3>();
		}

		std::pair<glm::vec3, glm::vec3> EditorLayer::transformToAABB(const TransformComponent& transform) const
		{
			auto [translation, rotation, scale] = transform.GetWorldComponents();
			scale.z = 0.0f; // 2D
			return { translation - (scale / 2.0f),translation + (scale / 2.0f) };
		}

		void EditorLayer::displayStats()
		{
			if (ImGui::Begin("Stats"))
			{
				const auto& stats = Renderer::GetStats();
				ImGui::Text("Draw Arrays: %d", stats.DrawArraysCount);
				ImGui::Text("Draw Indexed: %d", stats.DrawIndexedCount);
				ImGui::Text("Draw Instanced: %d", stats.DrawInstancedCount);
				ImGui::Text("Draw Fullscreen: %d", stats.DrawFullscreenCount);
				ImGui::Text("Draw Indirect: %d", stats.DrawIndirectCount);
				ImGui::Text("Commands Count: %d", stats.CommandsCount);
			}
			ImGui::End();
		}
	}
}