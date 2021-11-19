#include "EditorLayer.h"

#include <glm/glm.hpp>
#include <glm/gtx/transform.hpp>

#include <random>

namespace XYZ {

	struct TestVertex
	{
		glm::vec2 Position;
		glm::vec3 Color;
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
			{1, ShaderDataType::Float3, "inColor"}
		};
		const std::vector<TestVertex> vertices = {
			{{-0.5f, -0.5f}, {1.0f, 0.0f, 0.0f}},
			{{0.5f, -0.5f}, {0.0f, 1.0f, 0.0f}},
			{{0.5f, 0.5f}, {0.0f, 0.0f, 1.0f}},
			{{-0.5f, 0.5f}, {1.0f, 1.0f, 1.0f}}
		};
		const std::vector<uint16_t> indices = {
			0, 1, 2, 2, 3, 0
		};

		m_VertexBuffer  = VertexBuffer::Create(vertices.data(), vertices.size() * sizeof(TestVertex));
		m_IndexBuffer   = IndexBuffer::Create(indices.data(), indices.size(), IndexType::Uint16);
		m_Pipeline		= Pipeline::Create({ m_Shader, layout, m_RenderPass });
		m_RenderCommandBuffer = context->GetRenderCommandBuffer();
		m_UniformBufferSet = UniformBufferSet::Create(Renderer::GetConfiguration().FramesInFlight);
		m_UniformBufferSet->Create(sizeof(TestCamera), 0, 0);
		m_UniformBufferSet->Create(sizeof(TestCamera), 1, 1);
		m_UniformBufferSet->CreateDescriptors(m_Shader);

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
		Renderer::RenderGeometry(m_RenderCommandBuffer, m_Pipeline, m_UniformBufferSet, m_VertexBuffer, m_IndexBuffer);
		Renderer::EndRenderPass(m_RenderCommandBuffer);
		m_RenderCommandBuffer->End();
	}

	void EditorLayer::OnEvent(Event& event)
	{			
		
	}

	void EditorLayer::OnImGuiRender()
	{
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

	void EditorLayer::gpuParticleExample(SceneEntity entity)
	{
		auto &particleComponent = entity.EmplaceComponent<ParticleComponentGPU>();	
		auto particleMaterial = Ref<ParticleMaterial>::Create(50, Shader::Create("Assets/Shaders/Particle/ComputeParticleShader.glsl"));
		
		particleComponent.System = Ref<ParticleSystem>::Create(particleMaterial);
		particleComponent.System->m_Renderer->m_Material = Ref<Material>::Create(Shader::Create("Assets/Shaders/Particle/ParticleShader.glsl"));
		particleComponent.System->m_Renderer->m_Material->SetTexture("u_Texture", Texture2D::Create({}, "Assets/Textures/cosmic.png"));
		particleComponent.System->m_Renderer->m_Material->SetRenderQueueID(1);
		
		std::vector<ParticleData> particleData;
		std::vector<ParticleSpecification> particleSpecification;
		
		std::random_device dev;
		std::mt19937 rng(dev());
		std::uniform_real_distribution<double> dist(-1.0, 1.0); // distribution in range [1, 6]
		for (size_t i = 0; i < particleMaterial->GetMaxParticles(); ++i)
		{
			ParticleData data;
			data.Color    = glm::vec4(1.0f);
			data.Position = glm::vec2(0.0f, 0.0f);
			data.Size	  = glm::vec2(0.2f);
			data.Rotation = 0.0f;
			particleData.push_back(data);
		
			ParticleSpecification specs;
			specs.StartColor = glm::vec4(1.0f, 1.0f, 1.0f, 1.0f);
			specs.StartPosition = glm::vec2(0.0f, 0.0f);
			specs.StartSize		= glm::vec2(0.2f);
			specs.StartVelocity = glm::vec2(dist(rng), 1.0f);
		
			particleSpecification.push_back(specs);
		}
		particleComponent.System->SetSpawnRate(3.0f);
		
		particleMaterial->Set("u_Force", glm::vec2(-1.0f, 0.0f));
		
		// Main module
		particleMaterial->Set("u_MainModule.Repeat", 1);
		particleMaterial->Set("u_MainModule.LifeTime", 3.0f);
		particleMaterial->Set("u_MainModule.Speed", 1.0f);
		// Color module
		particleMaterial->Set("u_ColorModule.StartColor", glm::vec4(0.5f));
		particleMaterial->Set("u_ColorModule.EndColor",   glm::vec4(1.0f));
		// Size module
		particleMaterial->Set("u_SizeModule.StartSize", glm::vec2(0.1f));
		particleMaterial->Set("u_SizeModule.EndSize", glm::vec2(3.0f));
		// Texture animation module
		particleMaterial->Set("u_TextureModule.TilesX", 1);
		particleMaterial->Set("u_TextureModule.TilesY", 1);
		
		particleMaterial->SetBufferData("buffer_Data", particleData.data(), (uint32_t)particleData.size(), (uint32_t)sizeof(ParticleData));
		particleMaterial->SetBufferData("buffer_Specification", particleSpecification.data(), (uint32_t)particleSpecification.size(), (uint32_t)sizeof(ParticleSpecification));
	}

	void EditorLayer::cpuParticleExample(SceneEntity entity)
	{	
		uint32_t numParticles = 1000;
		auto& meshComponent = entity.EmplaceComponent<MeshComponent>();
		meshComponent.Mesh = MeshFactory::CreateInstancedQuad(
			glm::vec3(1.0f), 
			{ 
				{ 0, XYZ::ShaderDataType::Float3, "a_Position" },
				{ 1, XYZ::ShaderDataType::Float2, "a_TexCoord" }
			},
			{
				{ 2, XYZ::ShaderDataType::Float4, "a_IColor",     1 },
				{ 3, XYZ::ShaderDataType::Float3, "a_IPosition",  1 },
				{ 4, XYZ::ShaderDataType::Float3, "a_ISize",      1 },
				{ 5, XYZ::ShaderDataType::Float4, "a_IAxis",      1 },
				{ 6, XYZ::ShaderDataType::Float2, "a_ITexOffset", 1 }
			}, numParticles);

		auto& particleComponentCPU = entity.EmplaceComponent<ParticleComponentCPU>();
		particleComponentCPU.System.SetMaxParticles(numParticles);

		auto shaderLibrary = Renderer::GetShaderLibrary();
		Ref<Material> material = Ref<Material>::Create(shaderLibrary->Get("ParticleShaderCPU"));
		material->SetTexture("u_Texture", Texture2D::Create({}, "Assets/Textures/checkerboard.png"));
		material->Set("u_Tiles", glm::vec2(1.0f, 1.0f));
		material->SetRenderQueueID(1);
		meshComponent.Mesh->SetMaterial(material);

		particleComponentCPU.System.SetSceneEntity(entity);
		particleComponentCPU.System.Play();
		auto& lightStorage = m_Scene->GetECS().GetStorage<PointLight2D>();
		if (lightStorage.Size())
		{
			SceneEntity lightEntity(lightStorage.GetEntityAtIndex(0), m_Scene.Raw());
			{
				auto moduleData = particleComponentCPU.System.GetModuleData();
				moduleData->m_LightModule.m_LightEntity = lightEntity;
				moduleData->m_LightModule.m_TransformEntity = entity;
			}
			//auto &newMeshComponent = lightEntity.EmplaceComponent<MeshComponent>();
			//newMeshComponent.Mesh = entity.GetComponent<MeshComponent>().Mesh;
			//auto &particleComponent = lightEntity.EmplaceComponent<ParticleComponentCPU>();
			//particleComponent.System.SetMaxParticles(numParticles);
			//particleComponent.System.Play();
			//{
			//	auto moduleData = particleComponent.System.GetModuleData();
			//	moduleData->m_LightModule.m_LightEntity = lightEntity;
			//	moduleData->m_LightModule.m_TransformEntity = entity;
			//}
		}
	}

	void EditorLayer::animationExample(SceneEntity entity)
	{
		/*
		auto& animator = entity.EmplaceComponent<AnimatorComponent>();
		//animator.Animation = AssetManager::GetAsset<Animation>(AssetManager::GetAssetHandle("Assets/Animations/havko.anim"));
		Ref<Animation> animation = AssetManager::CreateAsset<Animation>("havko.anim", "Assets/Animations");
		animator.Animator = Ref<Animator>::Create();
		animator.Animator->SetAnimation(animation);
		animator.Animator->SetSceneEntity(entity);

		m_AnimationEditor.SetScene(m_Scene);
		m_AnimationEditor.SetContext(animator.Animator);
		*/
	}
}