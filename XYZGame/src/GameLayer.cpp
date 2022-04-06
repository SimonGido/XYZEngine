#include "GameLayer.h"

#include <glm/glm.hpp>
#include <glm/gtx/transform.hpp>

#include <random>

namespace XYZ {

	struct GPUTimeQueries
	{
		uint32_t GPUTime = 0;
		uint32_t Renderer2DPassQuery = 0;

		static constexpr uint32_t Count() { return sizeof(GPUTimeQueries) / sizeof(uint32_t); }
	};

	static Ref<RenderPass> CreateRenderPass()
	{
		FramebufferSpecification compFramebufferSpec;
		compFramebufferSpec.ClearColor = { 0.1f, 0.1f, 0.1f, 1.0f };
		compFramebufferSpec.SwapChainTarget = true;
		compFramebufferSpec.ClearOnLoad = false;

		compFramebufferSpec.Attachments = { ImageFormat::RGBA };
		Ref<Framebuffer> framebuffer = Framebuffer::Create(compFramebufferSpec);

		RenderPassSpecification renderPassSpec;
		renderPassSpec.TargetFramebuffer = framebuffer;
		return RenderPass::Create(renderPassSpec);;
	}

	static Ref<Pipeline> CreatePipeline(const Ref<Shader>& shader, const Ref<RenderPass>& renderPass)
	{
		PipelineSpecification specification;
		specification.Shader = shader;
		specification.Layouts = shader->GetLayouts();
		specification.RenderPass = renderPass;
		specification.Topology = PrimitiveTopology::Triangles;
		specification.DepthTest = true;
		return Pipeline::Create(specification);
	}

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
	
		m_CommandBuffer = RenderCommandBuffer::Create(0, "GameCommandBuffer");
		m_CommandBuffer->CreateTimestampQueries(GPUTimeQueries::Count());
		m_RenderPass = CreateRenderPass();

		std::vector<BufferLayout> layouts = { {
			{ ShaderDataType::Float3, "a_Position"},
			{ ShaderDataType::Float2, "a_TexCoord"}
		} };

		m_Material = Material::Create(Shader::Create("Assets/Voxel.glsl", layouts));
		m_MaterialInstance = Ref<MaterialInstance>::Create(m_Material);
		m_Pipeline = CreatePipeline(m_Material->GetShader(), m_RenderPass);
		m_StorageBufferSet = StorageBufferSet::Create(3);
		m_StorageBufferSet->Create(32 * 32 * 32 * sizeof(int), 0, 3);
		
		int voxels[32][32][32];
		for (int i = 0; i < 32; ++i)
		{
			for (int j = 0; j < 32; ++j)
			{
				for (int k = 0; k < 32; ++k)
				{
					voxels[i][j][k] = 1;
				}
			}
		}
		m_StorageBufferSet->Update(voxels, 32 * 32 * 32 * sizeof(int), 0, 3);
		m_UniformBufferSet = UniformBufferSet::Create(3);
		m_UniformBufferSet->Create(sizeof(UBCamera), 0, 0);

		m_TestQuadEntity = m_Scene->CreateEntity("TestQuad");

		m_TestQuadEntity.EmplaceComponent<SpriteRenderer>(
			Renderer::GetDefaultResources().DefaultQuadMaterial,
			Ref<SubTexture>::Create(Renderer::GetDefaultResources().WhiteTexture),
			glm::vec4(1.0f),
			0
		);

		m_TestQuadEntity.EmplaceComponent<PointLight2D>(glm::vec3(1.0f), 1.0f, 1.0f);

		Renderer::WaitAndRenderAll();
	}

	void GameLayer::OnDetach()
	{
	}
	void GameLayer::OnUpdate(Timestep ts)
	{
		m_Camera.OnUpdate(ts);
		m_CommandBuffer->Begin();

		UBCamera ub;
		ub.ViewProjection = m_Camera.GetViewProjection();
		ub.InverseView = glm::inverse(m_Camera.GetViewMatrix());

		const uint32_t currentFrame = Renderer::GetAPIContext()->GetCurrentFrame();
		m_UniformBufferSet->Get(0, 0, currentFrame)->Update(&ub, sizeof(UBCamera), 0);

		Renderer::BeginRenderPass(m_CommandBuffer, m_RenderPass, true);
		
		Renderer::BindPipeline(m_CommandBuffer, m_Pipeline, m_UniformBufferSet, m_StorageBufferSet, m_Material);
		Renderer::SubmitFullscreenQuad(m_CommandBuffer, m_Pipeline, m_MaterialInstance);
		Renderer::EndRenderPass(m_CommandBuffer);

		m_CommandBuffer->End();
		m_CommandBuffer->Submit();

		//m_Scene->OnUpdate(ts);
		//m_Scene->OnRenderEditor(m_SceneRenderer, m_Camera.GetViewProjection(), m_Camera.GetViewMatrix(), m_Camera.GetPosition());
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