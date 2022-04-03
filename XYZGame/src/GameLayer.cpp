#include "GameLayer.h"

#include <glm/glm.hpp>
#include <glm/gtx/transform.hpp>

#include <random>

namespace XYZ {

	GameLayer::GameLayer()
		:
		m_Camera(30.0f, 1.778f, 0.1f, 1000.0f)
	{
	}

	GameLayer::~GameLayer()
	{
		
	}

	void GameLayer::OnAttach()
	{
		Application::Get().GetImGuiLayer()->EnableDockspace(false);
		const uint32_t windowWidth = Application::Get().GetWindow().GetWidth();
		const uint32_t windowHeight = Application::Get().GetWindow().GetHeight();

		
		m_Scene = Ref<Scene>::Create("Game");
		m_Scene->SetViewportSize(windowWidth, windowHeight);
		m_SceneRenderer = Ref<SceneRenderer>::Create(m_Scene, SceneRendererSpecification{ true });
		m_Camera.SetViewportSize((float)windowWidth, (float)windowHeight);
	

		m_TestQuadEntity = m_Scene->CreateEntity("TestQuad");

		m_TestQuadEntity.EmplaceComponent<SpriteRenderer>(
			Renderer::GetDefaultResources().DefaultQuadMaterial,
			Ref<SubTexture>::Create(Renderer::GetDefaultResources().WhiteTexture),
			glm::vec4(1.0f),
			0
		);

		m_TestQuadEntity.EmplaceComponent<PointLight2D>(glm::vec3(1.0f), 1.0f, 1.0f);

		std::vector<Triangle> triangles;
		MarchingCubes::PerlinPolygonize(glm::vec3(-50.0f), glm::vec3(50.0f), 10, 10, 10, triangles);
		
		std::vector<Vertex> vertices;
		std::vector<uint32_t> indices;
		uint32_t counter = 0;
		for (const auto& tr : triangles)
		{
			vertices.push_back({ tr[2], glm::vec2(0.0f) });
			vertices.push_back({ tr[1], glm::vec2(0.0f) });
			vertices.push_back({ tr[0], glm::vec2(0.0f) });
			indices.push_back(counter++);
			indices.push_back(counter++);
			indices.push_back(counter++);
		}
		Ref<MeshSource> meshSource = Ref<MeshSource>::Create(vertices, indices);
		Ref<Mesh> mesh = Ref<Mesh>::Create(meshSource);

		Ref<ShaderAsset> meshShaderAsset = AssetManager::GetAsset<ShaderAsset>("Resources/Shaders/MeshShader.shader");
		auto material = Ref<MaterialAsset>::Create(meshShaderAsset);
		material->SetTexture("u_Texture", Renderer::GetDefaultResources().WhiteTexture);
		m_TestQuadEntity.EmplaceComponent<MeshComponent>(mesh, material);

		Renderer::WaitAndRenderAll();
	}

	void GameLayer::OnDetach()
	{
	}
	void GameLayer::OnUpdate(Timestep ts)
	{
		m_Camera.OnUpdate(ts);
		m_Scene->OnUpdate(ts);
		
		m_Scene->OnRenderEditor(m_SceneRenderer, m_Camera.GetViewProjection(), m_Camera.GetViewMatrix(), m_Camera.GetPosition());
	}

	void GameLayer::OnEvent(Event& event)
	{
		m_Camera.OnEvent(event);
	}

	void GameLayer::OnImGuiRender()
	{
		m_SceneRenderer->OnImGuiRender();
	}

	bool GameLayer::onMouseButtonPress(MouseButtonPressEvent& event)
	{
		return false;
	}
	bool GameLayer::onMouseButtonRelease(MouseButtonReleaseEvent& event)
	{
		return false;
	}
	bool GameLayer::onWindowResize(WindowResizeEvent& event)
	{
		return false;
	}

	bool GameLayer::onKeyPress(KeyPressedEvent& event)
	{
		return false;
	}

	bool GameLayer::onKeyRelease(KeyReleasedEvent& event)
	{
		return false;
	}

	void GameLayer::displayStats()
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