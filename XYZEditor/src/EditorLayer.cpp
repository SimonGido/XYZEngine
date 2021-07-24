#include "EditorLayer.h"

#include <glm/glm.hpp>
#include <glm/gtx/transform.hpp>

#include <random>

namespace XYZ {

	EditorLayer::EditorLayer()
		:
		m_EditorOpen{ false, true }
	{			
	}

	EditorLayer::~EditorLayer()
	{
	}

	void EditorLayer::OnAttach()
	{
		ScriptEngine::Init("Assets/Scripts/XYZScript.dll");
		m_Scene = AssetManager::GetAsset<Scene>(AssetManager::GetAssetHandle("Assets/Scenes/NewScene.xyz"));
		m_SceneHierarchy.SetContext(m_Scene);
		m_ScenePanel.SetContext(m_Scene);	
		ScriptEngine::SetSceneContext(m_Scene);

		uint32_t windowWidth = Application::Get().GetWindow().GetWidth();
		uint32_t windowHeight = Application::Get().GetWindow().GetHeight();
		SceneRenderer::SetViewportSize(windowWidth, windowHeight);
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

		auto entity = m_Scene->GetEntityByName("Scary Entity");
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
		m_Timestep = ts;
		if (m_Scene->GetState() == SceneState::Play)
		{
			m_Scene->OnUpdate(ts);
			m_Scene->OnRender();
		}
		else
		{
			auto& editorCamera = m_ScenePanel.GetEditorCamera();
			m_Scene->OnRenderEditor(editorCamera, ts);

			EditorRenderer::BeginPass(SceneRenderer::GetFinalRenderPass(), editorCamera.GetViewProjection(), editorCamera.GetPosition());
			EditorRenderer::EndPass();
		}

		m_ScenePanel.OnUpdate(ts);
		m_SpriteEditor.OnUpdate(ts);
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
		m_Inspector.OnImGuiRender();
		m_SceneHierarchy.OnImGuiRender();
		m_ScenePanel.OnImGuiRender();
		m_SpriteEditor.OnImGuiRender(m_EditorOpen[SpriteEditor]);
		m_AnimationEditor.OnImGuiRender(m_EditorOpen[AnimationEditor]);
		m_AssetBrowser.OnImGuiRender();
		displayStats();
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
			ImGui::Text("Timestep: %f ms", m_Timestep.GetMilliseconds());
			ImGui::Text("FPS: %f", 1.0f / m_Timestep.GetSeconds());
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
		particleComponent.System->m_Renderer.Material = Ref<Material>::Create(Shader::Create("Assets/Shaders/Particle/ParticleShader.glsl"));
		particleComponent.System->m_Renderer.Material->Set("u_Texture", Texture2D::Create({}, "Assets/Textures/cosmic.png"));
		particleComponent.System->m_Renderer.Material->SetRenderQueueID(1);
		
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
		auto& particleComponentCPU = entity.EmplaceComponent<ParticleComponentCPU>();
		particleComponentCPU.System = Ref<ParticleSystemCPU>::Create(100);

		particleComponentCPU.System->GetRenderer().Material = Ref<Material>::Create(Shader::Create("Assets/Shaders/Particle/ParticleShaderCPU.glsl"));
		particleComponentCPU.System->GetRenderer().Material->Set("u_Texture", Texture2D::Create({}, "Assets/Textures/cosmic.png"));
		particleComponentCPU.System->GetRenderer().Material->SetRenderQueueID(1);
		particleComponentCPU.System->Play();

		Ref<ParticleEmitterCPU> emitter = Ref<ParticleEmitterCPU>::Create();
		emitter->AddGenerator(Ref<ParticleShapeGenerator>::Create());
		emitter->AddGenerator(Ref<ParticleLifeGenerator>::Create());
		emitter->AddGenerator(Ref<ParticleRandomVelocityGenerator>::Create());
		particleComponentCPU.System->AddEmitter(emitter);
		particleComponentCPU.System->AddUpdater(Ref<BasicTimerUpdater>::Create());
		particleComponentCPU.System->AddUpdater(Ref<PositionUpdater>::Create());


		Ref<LightUpdater> lightUpdater = Ref<LightUpdater>::Create();
		particleComponentCPU.System->AddUpdater(lightUpdater);
		auto& lightStorage = m_Scene->GetECS().GetStorage<PointLight2D>();
		if (lightStorage.Size())
		{
			SceneEntity lightEntity(lightStorage.GetEntityAtIndex(0), m_Scene.Raw());
			lightUpdater->SetLightEntity(lightEntity);
			auto& another = lightEntity.EmplaceComponent<ParticleComponentCPU>();
			another.System = Ref<ParticleSystemCPU>::Create(100);
			another.System->GetRenderer().Material = Ref<Material>::Create(Shader::Create("Assets/Shaders/Particle/ParticleShaderCPU.glsl"));
			another.System->GetRenderer().Material->Set("u_Texture", Texture2D::Create({}, "Assets/Textures/cosmic.png"));
			another.System->GetRenderer().Material->SetRenderQueueID(1);
			another.System->Play();

			another.System->AddEmitter(emitter);
			another.System->AddUpdater(Ref<BasicTimerUpdater>::Create());
			another.System->AddUpdater(Ref<PositionUpdater>::Create());


			Ref<LightUpdater> anotherLightUpdater = Ref<LightUpdater>::Create();
			anotherLightUpdater->SetLightEntity(lightEntity);
			anotherLightUpdater->SetTransformEntity(lightEntity);
			another.System->AddUpdater(anotherLightUpdater);
		}
		lightUpdater->SetTransformEntity(entity);
	}

	void EditorLayer::animationExample(SceneEntity entity)
	{
		auto& animator = entity.EmplaceComponent<AnimatorComponent>();
		animator.Animation = Ref<Animation>::Create(entity);
		m_AnimationEditor.SetContext(animator.Animation);
	}

}