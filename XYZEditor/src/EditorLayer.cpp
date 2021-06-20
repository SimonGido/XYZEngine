#include "EditorLayer.h"

#include <glm/glm.hpp>
#include <glm/gtx/transform.hpp>

#include <random>

namespace XYZ {

	EditorLayer::EditorLayer()
	{			
	}

	EditorLayer::~EditorLayer()
	{
	}

	void EditorLayer::OnAttach()
	{
		ScriptEngine::Init("Assets/Scripts/XYZScript.dll");
		m_Scene = AssetManager::GetAsset<Scene>(AssetManager::GetAssetHandle("Assets/Scenes/NewScene.xyz"));
		// m_Scene = AssetManager::CreateAsset<Scene>("NewScene.xyz", AssetType::Scene, AssetManager::GetDirectoryHandle("Assets/Scenes"), "New Scene");
		m_SceneHierarchy.SetContext(m_Scene);
		m_ScenePanel.SetContext(m_Scene);	
		ScriptEngine::SetSceneContext(m_Scene);

		uint32_t windowWidth = Application::Get().GetWindow().GetWidth();
		uint32_t windowHeight = Application::Get().GetWindow().GetHeight();
		SceneRenderer::SetViewportSize(windowWidth, windowHeight);
		m_Scene->SetViewportSize(windowWidth, windowHeight);		

		Renderer::WaitAndRender();


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


		auto entity = m_Scene->GetEntityByName("Scary Entity");
		auto &particleComponent = entity.EmplaceComponent<ParticleComponent>();
		particleComponent.RenderMaterial = Ref<Material>::Create(Shader::Create("Assets/Shaders/Particle/ParticleShader.glsl"));
		particleComponent.RenderMaterial->Set("u_Texture", Texture2D::Create({}, "Assets/Textures/cosmic.png"));
		particleComponent.Effect = Ref<ParticleMaterial>::Create(1000, Shader::Create("Assets/Shaders/Particle/ComputeParticleShader.glsl"));
		
		std::vector<ParticleData> particleData;
		std::vector<ParticleSpecification> particleSpecification;
		
		std::random_device dev;
		std::mt19937 rng(dev());
		std::uniform_real_distribution<double> dist(-1.0, 1.0); // distribution in range [1, 6]
		for (size_t i = 0; i < particleComponent.Effect->GetMaxParticles(); ++i)
		{
			ParticleData data;
			data.Color    = glm::vec4(1.0f);
			data.Position = glm::vec2(0.0f, 0.0f);
			data.TexCoord = glm::vec2(0.0f);
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
		particleComponent.Effect->SetSpawnRate(40.0f);
		particleComponent.Effect->Set("u_NumBoxColliders", 0);
		particleComponent.Effect->Set("u_MaxParticles", 1000);
		
		particleComponent.Effect->Set("u_Force", glm::vec2(-1.0f, 0.0f));
		
		// Main module
		particleComponent.Effect->Set("u_MainModule.Repeat", 1);
		particleComponent.Effect->Set("u_MainModule.LifeTime", 3.0f);
		particleComponent.Effect->Set("u_MainModule.Speed", 1.0f);
		// Color module
		particleComponent.Effect->Set("u_ColorModule.StartColor", glm::vec4(0.5f));
		particleComponent.Effect->Set("u_ColorModule.EndColor",   glm::vec4(1.0f));
		// Size module
		particleComponent.Effect->Set("u_SizeModule.StartSize", glm::vec2(0.1f));
		particleComponent.Effect->Set("u_SizeModule.EndSize", glm::vec2(3.0f));
		// Texture animation module
		particleComponent.Effect->Set("u_TextureModule.TilesX", 4);
		particleComponent.Effect->Set("u_TextureModule.TilesY", 4);

		//particleComponent.Effect->SetBufferSize("buffer_Data", particleData.size() * sizeof(ParticleData));
		//particleComponent.Effect->SetBufferSize("buffer_Specification", particleSpecification.size() * sizeof(ParticleSpecification));
		particleComponent.Effect->SetBufferData("buffer_Data", particleData.data(), particleData.size(), sizeof(ParticleData));
		particleComponent.Effect->SetBufferData("buffer_Specification", particleSpecification.data(), particleSpecification.size(), sizeof(ParticleSpecification));
	}	

	void EditorLayer::OnDetach()
	{
		ScriptEngine::Shutdown();
		AssetSerializer::SerializeAsset(m_Scene);		
	}
	void EditorLayer::OnUpdate(Timestep ts)
	{		
		Renderer::SetClearColor(glm::vec4(0.1f, 0.1f, 0.1f, 1.0f));
		Renderer::Clear();

		m_ScenePanel.OnUpdate(ts);

		if (m_Scene->GetState() == SceneState::Play)
		{
			m_Scene->OnUpdate(ts);
			m_Scene->OnRender();
		}
		else
		{
			m_Scene->OnRenderEditor(m_ScenePanel.GetEditorCamera());
		}		
	}

	void EditorLayer::OnEvent(Event& event)
	{			
		EventDispatcher dispatcher(event);

		dispatcher.Dispatch<MouseButtonPressEvent>(Hook(&EditorLayer::onMouseButtonPress, this));
		dispatcher.Dispatch<MouseButtonReleaseEvent>(Hook(&EditorLayer::onMouseButtonRelease, this));	
		dispatcher.Dispatch<WindowResizeEvent>(Hook(&EditorLayer::onWindowResize, this));
		dispatcher.Dispatch<KeyPressedEvent>(Hook(&EditorLayer::onKeyPress, this));
		
		m_ScenePanel.OnEvent(event);
		if (!event.Handled)
		{
			m_ScenePanel.GetEditorCamera().OnEvent(event);
		}
	}

	void EditorLayer::OnImGuiRender()
	{
		m_Inspector.OnImGuiRender();
		m_SceneHierarchy.OnImGuiRender();
		m_ScenePanel.OnImGuiRender();
		m_AssetBrowser.OnImGuiRender();
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

}