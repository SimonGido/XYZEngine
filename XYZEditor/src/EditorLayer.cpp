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
			Project::New();

			const float keepAliveAssetSeconds = 60;
			AssetManager::KeepAlive(keepAliveAssetSeconds);

			s_Data.Init();		
	
			m_CommandBuffer = PrimaryRenderCommandBuffer::Create(0, "Editor");
			m_CommandBuffer->CreateTimestampQueries(GPUTimeQueries::Count());

		
			createOverlayPipelines();
			auto scene = Ref<Scene>::Create("Temporary");
			ScriptEngine::SetSceneContext(scene);

			m_EditorManager.SetSceneContext(scene);
			auto consolePanel = m_EditorManager.RegisterPanel<Editor::EditorConsolePanel>("ConsolePanel");
			EditorLogger::Init(consolePanel->GetStream());
			ScriptEngine::SetLogger(EditorLogger::GetLogger());

			//m_EditorManager.RegisterPanel<Editor::ScenePanel>("ScenePanel");
			m_EditorManager.RegisterPanel<Editor::VoxelPanel>("VoxelPanel");
			m_EditorManager.RegisterPanel<Editor::InspectorPanel>("InspectorPanel");
			m_EditorManager.RegisterPanel<Editor::SceneHierarchyPanel>("SceneHierarchyPanel");
			m_EditorManager.RegisterPanel<Editor::ImGuiStylePanel>("ImGuiStylePanel");
			m_EditorManager.RegisterPanel<Editor::AssetManagerViewPanel>("AssetManagerViewPanel");
			m_EditorManager.RegisterPanel<Editor::AssetBrowser>("AssetBrowser");
			m_EditorManager.RegisterPanel<Editor::ScriptPanel>("ScriptPanel");
			m_EditorManager.RegisterPanel<Editor::PluginPanel>("PluginPanel");


			auto voxelPanel = m_EditorManager.GetPanel<Editor::VoxelPanel>("VoxelPanel");

			voxelPanel->SetVoxelRenderer(Ref<VoxelRenderer>::Create());

			m_EditorCamera = &voxelPanel->GetEditorCamera();

			Renderer::WaitAndRenderAll();

		
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
			PluginManager::Update(ts);
			m_EditorManager.OnUpdate(ts);

		}

		void EditorLayer::OnEvent(Event& event)
		{
			
			m_EditorManager.OnEvent(event);
		}

		void EditorLayer::OnImGuiRender()
		{
			ImGui::SetCurrentContext(XYZ::UI::GetImGuiContext());
			
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
			
		}

		void EditorLayer::renderColliders()
		{
			
		}

		void EditorLayer::renderCameras()
		{
			
		}

		void EditorLayer::renderLights()
		{
			
		}

		void EditorLayer::renderSelected()
		{
			
		}


		void EditorLayer::createOverlayPipelines()
		{
			
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