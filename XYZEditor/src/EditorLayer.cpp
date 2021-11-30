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
		m_ScenePanel.SetContext(m_Scene);
		m_SceneHierarchyPanel.SetContext(m_Scene);

		m_SceneRenderer = Ref<SceneRenderer>::Create(m_Scene, SceneRendererSpecification());

		Renderer::WaitAndRenderAll();
	}
	
	void EditorLayer::OnDetach()
	{
	}
	void EditorLayer::OnUpdate(Timestep ts)
	{
		auto& editorCamera = m_ScenePanel.GetEditorCamera();

		if (m_Scene->GetState() == SceneState::Edit)
		{
			m_ScenePanel.OnUpdate(ts);
			m_Scene->OnRenderEditor(m_SceneRenderer, editorCamera, ts);
		}
		else
		{
			m_Scene->OnUpdate(ts);
			m_Scene->OnRender(m_SceneRenderer);
		}

		if (m_SelectedEntity != m_Scene->GetSelectedEntity())
		{
			m_SelectedEntity = m_Scene->GetSelectedEntity();
			m_SceneEntityInspectorContext.SetContext(m_SelectedEntity);
			m_InspectorPanel.SetContext(&m_SceneEntityInspectorContext);
		}
	}

	void EditorLayer::OnEvent(Event& event)
	{			
		m_ScenePanel.OnEvent(event);
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

		m_ScenePanel.OnImGuiRender(m_SceneRenderer->GetFinalPassImage());
		m_SceneHierarchyPanel.OnImGuiRender(m_PanelsOpen[SceneHierarchy]);
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