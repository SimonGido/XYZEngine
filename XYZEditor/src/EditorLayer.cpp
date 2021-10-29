#include "EditorLayer.h"

#include <glm/glm.hpp>
#include <glm/gtx/transform.hpp>

#include <random>

namespace XYZ {

	EditorLayer::EditorLayer()
		:
		m_EditorOpen{ true, true }
	{			
	}

	EditorLayer::~EditorLayer()
	{
	}

	void EditorLayer::OnAttach()
	{
		ScriptEngine::Init("Assets/Scripts/XYZScript.dll");
		m_Scene = AssetManager::GetAsset<Scene>(AssetManager::GetAssetHandle("Assets/Scenes/NewScene.xyz"));
		m_SceneRenderer    = Ref<SceneRenderer>::Create();
		m_SceneRenderer2D  = Ref<Renderer2D>::Create();
		m_EditorRenderer = Ref<EditorRenderer>::Create();
		m_SceneRenderer->SetRenderer2D(m_SceneRenderer2D);

		m_SceneHierarchy.SetContext(m_Scene);
		m_ScenePanel.SetContext(m_Scene);	
		ScriptEngine::SetSceneContext(m_Scene);

		uint32_t windowWidth = Application::Get().GetWindow().GetWidth();
		uint32_t windowHeight = Application::Get().GetWindow().GetHeight();
		m_SceneRenderer->SetViewportSize(windowWidth, windowHeight);
		m_Scene->SetViewportSize(windowWidth, windowHeight);		

		m_AssetBrowser.SetAssetSelectedCallback([&](const Ref<Asset>& asset) {
			 m_AssetInspectorContext.SetContext(asset);
			 if (asset.Raw())
			 {
				 m_Inspector.SetContext(&m_AssetInspectorContext);
			 }
		});

		m_ScenePanel.SetEntitySelectedCallback([&](SceneEntity entity) {
			m_SceneEntityInspectorContext.SetContext(entity);
			if (entity)
			{
				m_Inspector.SetContext(&m_SceneEntityInspectorContext);
			}
		});

		m_SceneHierarchy.SetEntitySelectedCallback([&](SceneEntity entity) {
			m_SceneEntityInspectorContext.SetContext(entity);
			if (entity)
			{
				m_Inspector.SetContext(&m_SceneEntityInspectorContext);
			}
		});


		m_SpriteEditor.SetContext(AssetManager::GetAsset<Texture2D>(AssetManager::GetAssetHandle("Assets/Textures/player_sprite.png.tex")));


		auto entity = m_Scene->GetEntityByName("Body");
		//gpuParticleExample(entity);
		cpuParticleExample(entity);
		animationExample(entity);

		Renderer::WaitAndRender();
		Renderer::BlockRenderThread();
	}
	

	void EditorLayer::OnDetach()
	{
		ScriptEngine::Shutdown();
		AssetSerializer::SerializeAsset(m_Scene);		
	}
	void EditorLayer::OnUpdate(Timestep ts)
	{				
		if (m_Scene->GetState() == SceneState::Play)
		{
			m_Scene->OnUpdate(ts);
			m_Scene->OnRender(m_SceneRenderer);
		}
		else
		{
			auto& editorCamera = m_ScenePanel.GetEditorCamera();
			m_Scene->OnRenderEditor(m_SceneRenderer, m_EditorRenderer, editorCamera, ts);
		
			m_EditorRenderer->BeginPass(m_SceneRenderer->GetFinalRenderPass(), editorCamera.GetViewProjection(), editorCamera.GetPosition());		
			m_EditorRenderer->EndPass(m_SceneRenderer2D);
		}

		m_ScenePanel.OnUpdate(ts);
		m_SpriteEditor.OnUpdate(m_SceneRenderer2D, m_EditorRenderer, ts);
		m_AnimationEditor.OnUpdate(ts);
	}

	void EditorLayer::OnEvent(Event& event)
	{			
		EventDispatcher dispatcher(event);
		dispatcher.Dispatch<MouseButtonPressEvent>(Hook(&EditorLayer::onMouseButtonPress, this));
		dispatcher.Dispatch<MouseButtonReleaseEvent>(Hook(&EditorLayer::onMouseButtonRelease, this));	
		dispatcher.Dispatch<WindowResizeEvent>(Hook(&EditorLayer::onWindowResize, this));
		dispatcher.Dispatch<KeyPressedEvent>(Hook(&EditorLayer::onKeyPress, this));

		m_ScenePanel.OnEvent(event);
		m_SpriteEditor.OnEvent(event);
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
		m_Inspector.OnImGuiRender(m_EditorRenderer);
		m_SceneHierarchy.OnImGuiRender();
		m_SpriteEditor.OnImGuiRender(m_EditorOpen[SpriteEditor]);
		m_ScenePanel.OnImGuiRender(m_SceneRenderer->GetFinalColorBufferRendererID());
		m_AnimationEditor.OnImGuiRender(m_EditorOpen[AnimationEditor]);
		m_AssetBrowser.OnImGuiRender();
		displayStats();
		AssetManager::DisplayMemory();
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
		particleComponent.System->m_Renderer->m_Material->Set("u_Texture", Texture2D::Create({}, "Assets/Textures/cosmic.png"));
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
		meshComponent.Mesh = MeshFactory::CreateInstancedBox(
			glm::vec3(3.5f), 
			{ { 0, XYZ::ShaderDataComponent::Float3, "a_Position" } },
			{
				{ 1, XYZ::ShaderDataComponent::Float4, "a_IColor",    1 },
				{ 2, XYZ::ShaderDataComponent::Float4, "a_ITexCoord", 1 },
				{ 3, XYZ::ShaderDataComponent::Float3, "a_IPosition", 1 },
				{ 4, XYZ::ShaderDataComponent::Float3, "a_ISize",     1 },
				{ 5, XYZ::ShaderDataComponent::Float4, "a_IAxis",    1 }
			}, numParticles);

		auto& particleComponentCPU = entity.EmplaceComponent<ParticleComponentCPU>();
		particleComponentCPU.System.SetMaxParticles(numParticles);

		Ref<Material> material = Ref<Material>::Create(Shader::Create("Assets/Shaders/Particle/ParticleShaderCPU.glsl"));
		material->Set("u_Texture", Texture2D::Create({}, "Assets/Textures/cosmic.png"));
		material->SetRenderQueueID(1);
		meshComponent.Mesh->SetMaterial(material);

		particleComponentCPU.System.Play();
		particleComponentCPU.System.GetEmitter()->m_BurstEmitter.m_Bursts.emplace_back(50, 1.0f);
		particleComponentCPU.System.GetEmitter()->m_BurstEmitter.m_Bursts.emplace_back(50, 4.0f);

		auto& lightStorage = m_Scene->GetECS().GetStorage<PointLight2D>();
		if (lightStorage.Size())
		{
			SceneEntity lightEntity(lightStorage.GetEntityAtIndex(0), m_Scene.Raw());
			auto updateData = particleComponentCPU.System.GetUpdateData();
			updateData->m_LightUpdater.m_LightEntity = lightEntity;
			updateData->m_LightUpdater.m_TransformEntity = entity;
		}
		particleComponentCPU.System.Play();
	}

	void EditorLayer::animationExample(SceneEntity entity)
	{
		auto& animator = entity.EmplaceComponent<AnimatorComponent>();
		//animator.Animation = AssetManager::GetAsset<Animation>(AssetManager::GetAssetHandle("Assets/Animations/havko.anim"));
		Ref<Animation> animation = AssetManager::CreateAsset<Animation>("havko.anim", AssetType::Animation, AssetManager::GetDirectoryHandle("Assets/Animations"));
		animator.Animator = Ref<Animator>::Create();
		animator.Animator->SetAnimation(animation);
		animator.Animator->SetSceneEntity(entity);

		m_AnimationEditor.SetScene(m_Scene);
		m_AnimationEditor.SetContext(animator.Animator);
	}
}