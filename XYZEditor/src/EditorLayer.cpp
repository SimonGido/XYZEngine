#include "EditorLayer.h"

#include <glm/glm.hpp>
#include <glm/gtx/transform.hpp>

#include <random>

namespace XYZ {

	struct TestVertex
	{
		glm::vec2 Position;
		glm::vec3 Color;
		glm::vec2 TexCoord;
	};
	
	struct TestCamera
	{
		glm::mat4 ViewProjection;
		glm::mat4 View;
		glm::vec4 Position;
	};

	EditorLayer::EditorLayer()
		:
		m_PanelsOpen{ true, true, true }
	{			
	}

	EditorLayer::~EditorLayer()
	{
	}

	void EditorLayer::OnAttach()
	{
		m_Scene = Ref<Scene>::Create("TestScene");
		m_Scene->SetViewportSize(
			Application::Get().GetWindow().GetWidth(),
			Application::Get().GetWindow().GetHeight()
		);
		m_SceneRenderer = Ref<SceneRenderer>::Create(m_Scene, SceneRendererSpecification());

		m_EditorManager.SetSceneContext(m_Scene);
		m_EditorManager.RegisterPanel<Editor::ScenePanel>("ScenePanel");
		m_EditorManager.RegisterPanel<Editor::SceneHierarchyPanel>("SceneHierarchyPanel");
		m_EditorManager.GetPanel<Editor::ScenePanel>("ScenePanel")->SetSceneRenderer(m_SceneRenderer);

		Renderer::WaitAndRenderAll();
	}
	
	void EditorLayer::OnDetach()
	{
	}
	void EditorLayer::OnUpdate(Timestep ts)
	{
		m_EditorManager.OnUpdate(ts);

		if (m_SelectedEntity != m_Scene->GetSelectedEntity())
		{
			m_SelectedEntity = m_Scene->GetSelectedEntity();
			m_SceneEntityInspectorContext.SetContext(m_SelectedEntity);
			m_InspectorPanel.SetContext(&m_SceneEntityInspectorContext);
		}
	}

	void EditorLayer::OnEvent(Event& event)
	{			
		m_EditorManager.OnEvent(event);
	}

	void EditorLayer::OnImGuiRender()
	{
		m_SceneRenderer->OnImGuiRender();
		if (ImGui::BeginMenuBar())
		{
			if (ImGui::BeginMenu("File"))
			{
				if (ImGui::MenuItem("Open...", "Ctrl+O"))
				{
				}

				if (ImGui::MenuItem("Save As...", "Ctrl+Shift+S"))
				{
				}

				if (ImGui::MenuItem("Exit"))
				{
					Application::Get().Stop();
				}
				ImGui::EndMenu();
			}

			ImGui::EndMenuBar();
		}
		m_EditorManager.OnImGuiRender();
		m_InspectorPanel.OnImGuiRender(m_PanelsOpen[Inspector], m_SceneRenderer->GetRenderer2D());
		/*
		m_Inspector.OnImGuiRender(m_EditorRenderer);
		m_SpriteEditor.OnImGuiRender(m_EditorOpen[SpriteEditor]);
		m_ScenePanel.OnImGuiRender(m_SceneRenderer->GetFinalColorBufferRendererID());
		m_AnimationEditor.OnImGuiRender(m_EditorOpen[AnimationEditor]);
		m_AssetBrowser.OnImGuiRender();
		displayStats();
		AssetManager::DisplayMemory();
		*/
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