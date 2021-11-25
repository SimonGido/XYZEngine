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
		m_EditorOpen{ true, true },
		m_Camera(30.0f, 1.778f, 0.1f, 1000.0f)
	{			
	}

	EditorLayer::~EditorLayer()
	{
	}

	void EditorLayer::OnAttach()
	{
		m_Shader = Shader::Create("Assets/Shaders/VulkanTestVertexShader.glsl");
		Ref<APIContext> context = Renderer::GetAPIContext();
		m_RenderPass = context->GetRenderPass();
		BufferLayout layout {
			{0, ShaderDataType::Float2, "inPosition"},
			{1, ShaderDataType::Float3, "inColor"},
			{2, ShaderDataType::Float2, "inTexCoord"}
		};
		const std::vector<TestVertex> vertices = {
			{{-0.5f, -0.5f}, {1.0f, 1.0f, 1.0f}, {0.0f, 0.0f}},
			{{ 0.5f, -0.5f}, {1.0f, 1.0f, 1.0f}, {1.0f, 0.0f}},
			{{ 0.5f,  0.5f}, {1.0f, 1.0f, 1.0f}, {1.0f, 1.0f}},
			{{-0.5f,  0.5f}, {1.0f, 1.0f, 1.0f}, {0.0f, 1.0f}}
		};
		const std::vector<uint16_t> indices = {
			0, 1, 2, 2, 3, 0
		};

		FramebufferSpecification specs;
		specs.Attachments.push_back({
			ImageFormat::RGBA32F
			});

		specs.ClearColor = { 0.0f, 1.0f, 0.0f, 1.0f };
		m_Framebuffer   = Framebuffer::Create(specs);

		m_RenderPass    = RenderPass::Create({ m_Framebuffer });
		m_VertexBuffer  = VertexBuffer::Create(vertices.data(), vertices.size() * sizeof(TestVertex));
		m_IndexBuffer   = IndexBuffer::Create(indices.data(), indices.size(), IndexType::Uint16);
		m_Pipeline		= Pipeline::Create({ m_Shader, layout, m_RenderPass });
		m_RenderCommandBuffer = RenderCommandBuffer::Create(Renderer::GetConfiguration().FramesInFlight);

		m_UniformBufferSet = UniformBufferSet::Create(Renderer::GetConfiguration().FramesInFlight);
		m_UniformBufferSet->Create(sizeof(TestCamera), 0, 0);
		m_UniformBufferSet->Create(sizeof(TestCamera), 1, 1);
		m_UniformBufferSet->CreateDescriptors(m_Shader);
		m_Texture = Texture2D::Create("Assets/Textures/1_ORK_head.png");
		m_Material = Material::Create(m_Shader);

		Renderer::WaitAndRenderAll();
		m_Material->Set("u_Texture", m_Texture);

		const uint32_t windowWidth = Application::Get().GetWindow().GetWidth();
		const uint32_t windowHeight = Application::Get().GetWindow().GetHeight();
		m_Camera.SetViewportSize((float)windowWidth, (float)windowHeight);
	}
	
	void EditorLayer::OnDetach()
	{
	}
	void EditorLayer::OnUpdate(Timestep ts)
	{
		TestCamera camera;
		camera.ViewProjection = m_Camera.GetViewProjection();
		camera.View = m_Camera.GetViewMatrix();
		camera.Position = glm::vec4(m_Camera.GetPosition(), 1.0f);

		uint32_t currentFrame = Renderer::GetAPIContext()->GetCurrentFrame();
		m_Camera.OnUpdate(ts);
		m_RenderCommandBuffer->Begin();
		m_UniformBufferSet->Get(0, 0, currentFrame)->Update(&camera, sizeof(TestCamera), 0);
		m_UniformBufferSet->Get(1, 1, currentFrame)->Update(&camera, sizeof(TestCamera), 0);
		
		Renderer::BeginRenderPass(m_RenderCommandBuffer, m_RenderPass, false);
		Renderer::RenderGeometry(m_RenderCommandBuffer, m_Pipeline, m_UniformBufferSet, m_Material, m_VertexBuffer, m_IndexBuffer);
		Renderer::EndRenderPass(m_RenderCommandBuffer);
		
		m_RenderCommandBuffer->End();
		m_RenderCommandBuffer->Submit();
	}

	void EditorLayer::OnEvent(Event& event)
	{			
		m_Camera.OnEvent(event);
	}

	void EditorLayer::OnImGuiRender()
	{
		if (ImGui::Begin("Scene"))
		{		
			UI::Image(m_Framebuffer->GetImage(), ImVec2(500, 500));
			//UI::Image(m_Texture->GetImage(), ImVec2(200, 200));
		}
		ImGui::End();

		/*
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
		m_Inspector.OnImGuiRender(m_EditorRenderer);
		m_SceneHierarchy.OnImGuiRender();
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