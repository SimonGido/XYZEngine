#include "EditorLayer.h"

#include "Editor/Event/EditorEvents.h"
#include "XYZ/Utils/Math/Math.h"
#include "XYZ/Scene/SceneEntityUtils.h"

#include <glm/glm.hpp>
#include <glm/gtx/transform.hpp>

#include <random>

namespace XYZ {
	namespace Editor {
		
		EditorLayer::EditorLayer()
		{
		}

		EditorLayer::~EditorLayer()
		{
			
		}

		void EditorLayer::OnAttach()
		{		
			s_Data.Init();		

			ScriptEngine::Init("Assets/Scripts/XYZScript.dll");

			m_Scene = AssetManager::GetAsset<Scene>("Assets/Scenes/Scene.xyz");

			ScriptEngine::SetSceneContext(m_Scene);

			m_Scene->SetViewportSize(
				Application::Get().GetWindow().GetWidth(),
				Application::Get().GetWindow().GetHeight()
			);

			m_SceneRenderer = Ref<SceneRenderer>::Create(m_Scene, SceneRendererSpecification());
			m_CameraTexture = Texture2D::Create("Resources/Editor/Camera.png");

			m_CommandBuffer = PrimaryRenderCommandBuffer::Create(0, "Editor");
			m_CommandBuffer->CreateTimestampQueries(GPUTimeQueries::Count());

			Ref<MaterialAsset> quadMaterialAsset   = Renderer::GetDefaultResources().RendererAssets.at("OverlayQuadMaterial").As<MaterialAsset>();;
			Ref<MaterialAsset> lineMaterialAsset   = Renderer::GetDefaultResources().RendererAssets.at("OverlayLineMaterial").As<MaterialAsset>();;
			Ref<MaterialAsset> circleMaterialAsset = Renderer::GetDefaultResources().RendererAssets.at("OverlayCircleMaterial").As<MaterialAsset>();;
		
			m_QuadMaterial	 = quadMaterialAsset->GetMaterial();
			m_LineMaterial	 = lineMaterialAsset->GetMaterial();
			m_CircleMaterial = circleMaterialAsset->GetMaterial();
			
			m_QuadMaterialInstance   = quadMaterialAsset->GetMaterialInstance();
			m_LineMaterialInstance   = lineMaterialAsset->GetMaterialInstance();
			m_CircleMaterialInstance = circleMaterialAsset->GetMaterialInstance();
			
			m_QuadMaterial->SetImageArray("u_Texture", m_CameraTexture->GetImage(), 0);			

			m_OverlayRenderer2D = Ref<Renderer2D>::Create(Renderer2DConfiguration{
				m_CommandBuffer,
				m_SceneRenderer->GetUniformBufferSet(),
			});
			createOverlayPipelines();

			m_EditorManager.SetSceneContext(m_Scene);
			auto consolePanel = m_EditorManager.RegisterPanel<Editor::EditorConsolePanel>("ConsolePanel");
			EditorLogger::Init(consolePanel->GetStream());
			ScriptEngine::SetLogger(EditorLogger::GetLogger());

			m_Scene->CreateParticleTest();

			m_EditorManager.RegisterPanel<Editor::ScenePanel>("ScenePanel");
			m_EditorManager.RegisterPanel<Editor::InspectorPanel>("InspectorPanel");
			m_EditorManager.RegisterPanel<Editor::SceneHierarchyPanel>("SceneHierarchyPanel");
			m_EditorManager.RegisterPanel<Editor::ImGuiStylePanel>("ImGuiStylePanel");
			m_EditorManager.RegisterPanel<Editor::AssetManagerViewPanel>("AssetManagerViewPanel");
			m_EditorManager.RegisterPanel<Editor::AssetBrowser>("AssetBrowser");
			m_EditorManager.RegisterPanel<Editor::ScriptPanel>("ScriptPanel");
			//m_EditorManager.RegisterPanel<Editor::ParticleEditorGPU>("ParticleEditorGPU");

			std::shared_ptr<Editor::ScenePanel> scenePanel = m_EditorManager.GetPanel<Editor::ScenePanel>("ScenePanel");
		
			scenePanel->SetSceneRenderer(m_SceneRenderer);
			m_EditorCamera = &scenePanel->GetEditorCamera();

			Renderer::WaitAndRenderAll();

			const float keepAliveAssetSeconds = 60;
			AssetManager::KeepAlive(keepAliveAssetSeconds);

		}

		void EditorLayer::OnDetach()
		{
			s_Data.Shutdown();
			AssetManager::SerializeAll();

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
			if (event.GetEventType() == EventType::Editor)
			{
				EditorEvent& editorEvent = (EditorEvent&)event;
				if (editorEvent.GetEditorEventType() == EditorEventType::SceneLoaded)
				{
					SceneLoadedEvent& sceneLoadedEvent = (SceneLoadedEvent&)editorEvent;
					AssetManager::Serialize(m_Scene->GetHandle());
					m_Scene = sceneLoadedEvent.GetScene();
					m_SceneRenderer->SetScene(m_Scene);
					ScriptEngine::SetSceneContext(m_Scene);
					m_SelectedEntity = {};
				}
			}
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
			
			Renderer::BeginRenderPass(m_CommandBuffer, m_SceneRenderer->GetFinalRenderPass(), false, false);	
			
			m_OverlayRenderer2D->BeginScene(m_EditorCamera->GetViewMatrix());	
			renderSelected();
			renderColliders();
			renderCameras();
			renderLights();	
			Renderer::BindPipeline(m_CommandBuffer, m_OverlayQuadPipeline, m_SceneRenderer->GetUniformBufferSet(), nullptr, m_QuadMaterial);
			m_OverlayRenderer2D->FlushQuads(m_OverlayQuadPipeline, m_QuadMaterialInstance, true);
			
			Renderer::BindPipeline(m_CommandBuffer, m_OverlayLinePipeline, m_SceneRenderer->GetUniformBufferSet(), nullptr, m_LineMaterial);
			m_OverlayRenderer2D->FlushLines(m_OverlayLinePipeline, m_LineMaterialInstance, true);
			
			Renderer::BindPipeline(m_CommandBuffer, m_OverlayCirclePipeline, m_SceneRenderer->GetUniformBufferSet(), nullptr, m_CircleMaterial);
			m_OverlayRenderer2D->FlushFilledCircles(m_OverlayCirclePipeline, m_CircleMaterialInstance, true);
			
			m_OverlayRenderer2D->EndScene();
			Renderer::EndRenderPass(m_CommandBuffer);

			m_CommandBuffer->EndTimestampQuery(m_GPUTimeQueries.GPUTime);
			
			m_CommandBuffer->End();
			m_CommandBuffer->Submit();

			float gpuTime = m_CommandBuffer->GetExecutionGPUTime(Renderer::GetCurrentFrame(), m_GPUTimeQueries.GPUTime);
			Application::Get().GetPerformanceProfiler().PushMeasurement("EditorLayer::renderOverlay", gpuTime);
		}

		void EditorLayer::renderColliders()
		{
			if (m_ShowColliders)
			{
				auto& registry = m_Scene->GetRegistry();
				auto box2DColliderView = registry.view<TransformComponent, BoxCollider2DComponent>();
				for (auto entity : box2DColliderView)
				{
					auto &[transformComp, boxCollider] = box2DColliderView.get(entity);
					auto [translation, rotation, scale] = transformComp.GetWorldComponents();
					translation += glm::vec3(boxCollider.Offset, 0.0f);
					scale *= glm::vec3(boxCollider.Size, 1.0f);

					glm::mat4 transform = glm::translate(glm::mat4(1.0f), translation)
						* glm::rotate(glm::mat4(1.0f), rotation.z, glm::vec3(0.0f, 0.0f, 1.0f))
						* glm::scale(glm::mat4(1.0f), scale);

					m_OverlayRenderer2D->SubmitRect(transform, s_Data.Color[ED::Collider2D]);
				}

				auto circle2DColliderView = registry.view<TransformComponent, CircleCollider2DComponent>();
				for (auto entity : circle2DColliderView)
				{
					auto &[transformComp, circleCollider] = circle2DColliderView.get(entity);
					auto [translation, rotation, scale] = transformComp.GetWorldComponents();
					translation += glm::vec3(circleCollider.Offset, 0.0f);
					scale *= glm::vec3(circleCollider.Radius * 2.0f);
					m_OverlayRenderer2D->SubmitFilledCircle(translation, glm::vec2(scale.x, scale.y), 0.01f, s_Data.Color[ED::Collider2D]);
				}

				auto chain2DColliderView = registry.view<TransformComponent, ChainCollider2DComponent>();
				for (auto entity : chain2DColliderView)
				{
					auto& [transformComp, chainCollider] = chain2DColliderView.get(entity);
					if (!chainCollider.Points.empty())
					{
						glm::vec4 p = transformComp->WorldTransform * glm::vec4(chainCollider.Points[0], 0.0f, 1.0f);
						m_OverlayRenderer2D->SubmitFilledCircle(glm::vec3(p), glm::vec2(0.2f), 0.3f, s_Data.Color[ED::Collider2D]);
					}
					for (size_t i = 1; i < chainCollider.Points.size(); ++i)
					{
						glm::vec4 p0 = transformComp->WorldTransform * glm::vec4(chainCollider.Points[i - 1], 0.0f, 1.0f);
						glm::vec4 p1 = transformComp->WorldTransform * glm::vec4(chainCollider.Points[i], 0.0f, 1.0f);

						m_OverlayRenderer2D->SubmitLine(glm::vec3(p0), glm::vec3(p1), s_Data.Color[ED::Collider2D]);
						m_OverlayRenderer2D->SubmitFilledCircle(glm::vec3(p1), glm::vec2(0.2f), 0.3f, s_Data.Color[ED::Collider2D]);
					}
				}
			}
		}

		void EditorLayer::renderCameras()
		{
			if (m_ShowCameras)
			{
				auto& registry = m_Scene->GetRegistry();
				auto cameraView = registry.view<TransformComponent, CameraComponent>();
				for (auto entity : cameraView)
				{
					auto &[transform, camera] = cameraView.get(entity);
					auto [min, max] = cameraToAABB(transform, camera.Camera);
					auto [translation, rotation, scale] = transform.GetWorldComponents();
					m_OverlayRenderer2D->SubmitQuadBillboard(translation, glm::vec2(scale.x, scale.y), glm::vec4(0.0f, 0.0f, 1.0f, 1.0f), 0);
				}
			}
		}

		void EditorLayer::renderLights()
		{
			if (m_ShowLights)
			{
				auto& registry = m_Scene->GetRegistry();

				auto spotLight2DView = registry.view<TransformComponent, SpotLightComponent2D>();

				auto pointLight2DView = registry.view<TransformComponent, PointLightComponent2D>();
			}
		}

		void EditorLayer::renderSelected()
		{
			const SceneEntity selected = m_Scene->GetSelectedEntity();
			if (selected.IsValid() && selected != m_Scene->GetSceneEntity())
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
					auto aabb = SceneEntityAABB(selected);
					m_OverlayRenderer2D->SubmitAABB(aabb.Min, aabb.Max, s_Data.Color[ED::BoundingBox]);
				}
			}
		}


		void EditorLayer::createOverlayPipelines()
		{
			{
				PipelineSpecification spec;
				spec.RenderPass = m_SceneRenderer->GetFinalRenderPass();
				spec.Shader = m_QuadMaterial->GetShader();
				spec.Topology = PrimitiveTopology::Triangles;
				spec.DepthTest = true;
				spec.DepthWrite = true;
				m_OverlayQuadPipeline = Pipeline::Create(spec);
			}
			{
				PipelineSpecification spec;
				spec.RenderPass = m_SceneRenderer->GetFinalRenderPass();
				spec.Shader = m_CircleMaterial->GetShader();
				spec.Topology = PrimitiveTopology::Triangles;
				spec.DepthTest = true;
				spec.DepthWrite = true;
				m_OverlayCirclePipeline = Pipeline::Create(spec);
			}
			{
				PipelineSpecification spec;
				spec.RenderPass = m_SceneRenderer->GetFinalRenderPass();
				spec.Shader = m_LineMaterial->GetShader();
				spec.Topology = PrimitiveTopology::Lines;
				spec.DepthTest = true;
				spec.DepthWrite = true;
				m_OverlayLinePipeline = Pipeline::Create(spec);
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