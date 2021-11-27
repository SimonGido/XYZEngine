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
		m_EditorOpen{ true, true, true }
	{			
	}

	EditorLayer::~EditorLayer()
	{
	}

	void EditorLayer::OnAttach()
	{
		m_Scene = Ref<Scene>::Create("TestScene");
		m_ScenePanel.SetContext(m_Scene);
		m_SceneHierarchyPanel.SetContext(m_Scene);

		m_Texture = Texture2D::Create("Assets/Textures/1_ORK_head.png");
		m_SceneRenderer2D = Ref<Renderer2D>::Create(Renderer2DSpecification());

		Renderer::WaitAndRenderAll();
	}
	
	void EditorLayer::OnDetach()
	{
	}
	void EditorLayer::OnUpdate(Timestep ts)
	{
		auto& camera = m_ScenePanel.GetEditorCamera();
		camera.OnUpdate(ts);

		m_SceneRenderer2D->BeginScene(camera.GetViewProjection());
		m_SceneRenderer2D->SubmitQuad(glm::vec3(0.0f), glm::vec2(1.0f), glm::vec4(0.0f, 0.0f, 1.0f, 1.0f), m_Texture, glm::vec4(1.0f));
		m_SceneRenderer2D->SubmitQuad(glm::vec3(3.0f), glm::vec2(1.0f), glm::vec4(0.0f, 0.0f, 1.0f, 1.0f), m_Texture, glm::vec4(1.0f));
		m_SceneRenderer2D->EndScene();
	}

	void EditorLayer::OnEvent(Event& event)
	{			
		m_ScenePanel.OnEvent(event);
	}

	void EditorLayer::OnImGuiRender()
	{
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
		auto renderPass = m_SceneRenderer2D->GetTargetRenderPass();
		auto frameBuffer = renderPass->GetSpecification().TargetFramebuffer;
		m_ScenePanel.OnImGuiRender(frameBuffer->GetImage());
		m_SceneHierarchyPanel.OnImGuiRender(m_EditorOpen[SceneHierarchy]);

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