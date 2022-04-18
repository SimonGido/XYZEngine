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

	static uint32_t RandomColor()
	{
		std::random_device rd; // obtain a random number from hardware
		std::mt19937 gen(rd()); // seed the generator
		std::uniform_int_distribution<> distr(0, 255); // define the range

		uint32_t result = 0;
		result |= static_cast<uint8_t>(distr(gen));		  // R
		result |= static_cast<uint8_t>(distr(gen)) << 8;  // G
		result |= static_cast<uint8_t>(distr(gen)) << 16; // B
		result |= static_cast<uint8_t>(distr(gen)) << 24; // A
	
		return result;
	}

	static uint32_t Index3D(int x, int y, int z, int width, int height)
	{
		return x + width * (y + height * z);
	}

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

	static glm::mat4 CameraFrustum(const EditorCamera& camera)
	{
		glm::vec3 Up = glm::vec3(0.0f, 1.0f, 0.0f);
		glm::vec3 Right = glm::vec3(1.0f, 0.0f, 0.0f);
		glm::vec3 Forward = glm::vec3(0.0f, 0.0f, 1.0f);

		//Forward = camera.GetForwardDirection();
		//Up = camera.GetUpDirection();
		//Right = camera.GetRightDirection();

		const float aspect = camera.GetAspectRatio();
		const float fov = glm::tan(glm::radians(camera.GetFOV() * 0.5f));

		
		const glm::vec3 goUp = Up * fov;
		const glm::vec3 goRight = Right * fov * aspect;

		const glm::vec3 topLeft		= -Forward - goRight + goUp;
		const glm::vec3 topRight	= -Forward + goRight + goUp;
		const glm::vec3 bottomRight = -Forward + goRight - goUp;
		const glm::vec3 bottomLeft	= -Forward - goRight - goUp;

		glm::mat4 frustum(1.0f);
		frustum[0] = { topLeft.x,     topLeft.y,     topLeft.z, 0.0f };
		frustum[1] = { topRight.x,    topRight.y,    topRight.z, 0.0f };
		frustum[2] = { bottomRight.x, bottomRight.y, bottomRight.z, 0.0f };
		frustum[3] = { bottomLeft.x,  bottomLeft.y,  bottomLeft.z, 0.0f };
		return frustum;
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

		m_Material = Material::Create(Shader::Create("Assets/RaymarchShader.glsl", layouts));
		m_MaterialInstance = Ref<MaterialInstance>::Create(m_Material);


		m_Pipeline = CreatePipeline(m_Material->GetShader(), m_RenderPass);
		m_StorageBufferSet = StorageBufferSet::Create(3);
		

		m_Voxels.resize(VOXEL_GRID_SIZE * VOXEL_GRID_SIZE * VOXEL_GRID_SIZE);

		m_StorageBufferSet->Create(m_Voxels.size() * sizeof(uint32_t), 0, 3);

		for (int i = 0; i < VOXEL_GRID_SIZE; ++i)
		{
			for (int j = 0; j < VOXEL_GRID_SIZE; ++j)
			{
				for (int k = 0; k < VOXEL_GRID_SIZE; ++k)
				{
					m_Voxels[Index3D(i, j, k, VOXEL_GRID_SIZE, VOXEL_GRID_SIZE)] = 0;
					if (j == 0)
						m_Voxels[Index3D(i, j, k, VOXEL_GRID_SIZE, VOXEL_GRID_SIZE)] = 1;
				}
			}
		}
		siv::PerlinNoise perlin{ 340 };

		
		generateVoxels();
		m_StorageBufferSet->Update(m_Voxels.data(), m_Voxels.size() * sizeof(uint32_t), 0, 3);
		m_SceneBufferSet = UniformBufferSet::Create(3);
		m_SceneBufferSet->Create(sizeof(UBScene), 0, 0);

		
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
		if (m_QueuedGenerate)
			generateVoxels();

		m_Camera.OnUpdate(ts);

		m_CommandBuffer->Begin();
	
		m_SceneUB.ViewProjection = m_Camera.GetViewProjection();
		m_SceneUB.InverseView = glm::inverse(m_Camera.GetViewMatrix());
		m_SceneUB.CameraFrustum = CameraFrustum(m_Camera);
		m_SceneUB.CameraPosition = glm::vec4(m_Camera.GetPosition(), 0.0f);

		const uint32_t currentFrame = Renderer::GetAPIContext()->GetCurrentFrame();
		m_SceneBufferSet->Get(0, 0, currentFrame)->Update(&m_SceneUB, sizeof(UBScene), 0);

		m_StorageBufferSet->Update(m_Voxels.data(), m_Voxels.size() * sizeof(uint32_t), 0, 3);
		Renderer::BeginRenderPass(m_CommandBuffer, m_RenderPass, true);

		Renderer::BindPipeline(m_CommandBuffer, m_Pipeline, m_SceneBufferSet, m_StorageBufferSet, m_Material);
		Renderer::SubmitFullscreenQuad(m_CommandBuffer, m_Pipeline, m_MaterialInstance);
		Renderer::EndRenderPass(m_CommandBuffer);

		m_CommandBuffer->End();
		m_CommandBuffer->Submit();
	}

	void GameLayer::OnEvent(Event& event)
	{
		m_Camera.OnEvent(event);
	}

	void GameLayer::OnImGuiRender()
	{
		m_SceneRenderer->OnImGuiRender();

		if (ImGui::Begin("Ray March"))
		{
			if (ImGui::DragFloat3("Light Direction", glm::value_ptr(m_SceneUB.LightDirection), 0.1f))
			{

			}
			if (ImGui::DragFloat3("Light Color", glm::value_ptr(m_SceneUB.LightColor), 0.1f))
			{

			}
			if (ImGui::DragFloat3("Chunk Position", glm::value_ptr(m_SceneUB.ChunkPosition), 0.1f))
			{

			}
			if (ImGui::DragInt("Max Traverse", (int*)&m_SceneUB.MaxTraverse))
			{
				
			}
			if (ImGui::DragFloat("Voxel Size", &m_SceneUB.VoxelSize, 0.1f))
			{

			}
			if (ImGui::DragInt("Seed", &m_Seed))
			{
				generateVoxels();
			}
			if (ImGui::DragInt("Octaves", &m_Octaves))
			{
				generateVoxels();
			}
			if (ImGui::DragFloat("Frequency", &m_Frequency, 0.05f))
			{
				generateVoxels();
			}
	
			float fov = m_Camera.GetFOV();
			if (ImGui::DragFloat("FOV", &fov))
			{
				m_Camera.SetFOV(fov);
			}
			const float aspect = m_Camera.GetAspectRatio();
			ImGui::Text("Aspect %f", aspect);
			ImGui::Text("Camera Position %f %f %f", m_Camera.GetPosition().x, m_Camera.GetPosition().y, m_Camera.GetPosition().z);
			if (m_Generating)
				ImGui::Text("Generating Voxels!!!");
		}
		ImGui::End();
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

	void GameLayer::generateVoxels()
	{
		if (m_Generating)
		{
			m_QueuedGenerate = true;
			return;
		}

		m_QueuedGenerate = false;
		m_Generating = true;
		Application::Get().GetThreadPool().PushJob([this]() {

			siv::PerlinNoise m_Noise{ (uint32_t) m_Seed};
			
			const double fx = (m_Frequency / VOXEL_GRID_SIZE);
			const double fy = (m_Frequency / VOXEL_GRID_SIZE);
			for (int i = 0; i < VOXEL_GRID_SIZE; ++i)
			{
				for (int j = 0; j < VOXEL_GRID_SIZE; ++j)
				{
					double x = i;
					double y = j;
					double val = m_Noise.octave2D(x * fx, y * fy, m_Octaves);
					int height = val * 16;
					for (int k = -16; k < height; ++k)
					{
						m_Voxels[Index3D(i, k + 16, j, VOXEL_GRID_SIZE, VOXEL_GRID_SIZE)] = RandomColor();
					}
				}
			}
			m_Generating = false;
		});	
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