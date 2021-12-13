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
		m_CameraTexture = Texture2D::Create("Resources/Editor/Camera.png");
		m_CommandBuffer = RenderCommandBuffer::Create(0, "Overlay");
		m_CommandBuffer->CreateTimestampQueries(GPUTimeQueries::Count());
		m_OverlayRenderer2D = Ref<Renderer2D>::Create(m_CommandBuffer);
		m_QuadMaterial = Material::Create(Shader::Create("Resources/Shaders/DefaultShader.glsl"));
		m_OverlayRenderer2D->SetQuadMaterial(m_QuadMaterial);
		m_LineMaterial = Material::Create(Shader::Create("Resources/Shaders/LineShader.glsl"));
		m_OverlayRenderer2D->SetLineMaterial(m_LineMaterial);

		m_EditorManager.SetSceneContext(m_Scene);
		m_EditorManager.RegisterPanel<Editor::ScenePanel>("ScenePanel");
		m_EditorManager.RegisterPanel<Editor::InspectorPanel>("InspectorPanel");
		m_EditorManager.RegisterPanel<Editor::SceneHierarchyPanel>("SceneHierarchyPanel");
		m_EditorManager.RegisterPanel<Editor::ImGuiStylePanel>("ImGuiStylePanel");
		m_EditorManager.RegisterPanel<Editor::AssetManagerViewPanel>("AssetManagerViewPanel");
		m_EditorManager.RegisterPanel<Editor::AssetBrowser>("AssetBrowser");
		
		Ref<Editor::ScenePanel> scenePanel = m_EditorManager.GetPanel<Editor::ScenePanel>("ScenePanel");
		scenePanel->SetSceneRenderer(m_SceneRenderer);
		m_EditorCamera = &scenePanel->GetEditorCamera();

		Renderer::WaitAndRenderAll();
	}
	
	void EditorLayer::OnDetach()
	{
	}
	void EditorLayer::OnUpdate(Timestep ts)
	{
		m_EditorManager.OnUpdate(ts);
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
		auto& ecs = m_Scene->GetECS();
		
		m_CommandBuffer->Begin();
		m_GPUTimeQueries.GPUTime = m_CommandBuffer->BeginTimestampQuery();
		m_OverlayRenderer2D->BeginScene(m_EditorCamera->GetViewProjection(), m_EditorCamera->GetViewMatrix());
		m_OverlayRenderer2D->SetTargetRenderPass(m_SceneRenderer->GetFinalRenderPass());

		auto box2DColliderView = ecs.CreateView<TransformComponent, BoxCollider2DComponent>();


		auto circle2DColliderView = ecs.CreateView<TransformComponent, CircleCollider2DComponent>();


		auto spotLight2DView = ecs.CreateView<TransformComponent, SpotLight2D>();
		
		
		auto pointLight2DView = ecs.CreateView<TransformComponent, PointLight2D>();

		auto cameraView = ecs.CreateView<TransformComponent, CameraComponent>();
		for (auto entity : cameraView)
		{
			auto [transform, camera] = cameraView.Get(entity);
			auto [min, max] = cameraToAABB(transform, camera.Camera);
			auto [translation, rotation, scale] = transform.GetWorldComponents();
			m_OverlayRenderer2D->SubmitAABB(min, max, glm::vec4(1.0f));
			m_OverlayRenderer2D->SubmitQuadBillboard(translation, glm::vec2(scale.x, scale.y), glm::vec4(0.0f, 0.0f, 1.0f, 1.0f), m_CameraTexture);
		}

		m_OverlayRenderer2D->EndScene(false);

		m_CommandBuffer->EndTimestampQuery(m_GPUTimeQueries.GPUTime);

		m_CommandBuffer->End();
		m_CommandBuffer->Submit();
	}

	std::pair<glm::vec3, glm::vec3> EditorLayer::cameraToAABB(const TransformComponent& transform, const SceneCamera& camera) const
	{
		auto [translation, rotation, scale] = transform.GetWorldComponents();
		if (camera.GetProjectionType() == CameraProjectionType::Orthographic)
		{
			float size = camera.GetOrthographicProperties().OrthographicSize;
			float aspect = (float)camera.GetViewportWidth() / (float)camera.GetViewportHeight();
			float width = size * aspect;
			float height = size;
		
			glm::vec3 bottomLeft = { translation.x - width / 2.0f,translation.y - height / 2.0f, translation.z };
			glm::vec3 topRight = { translation.x + width / 2.0f,translation.y + height / 2.0f, translation.z };
			return { bottomLeft, topRight };
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