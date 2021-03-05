#include "EditorLayer.h"

#include <glm/glm.hpp>
#include <glm/gtx/transform.hpp>
#include <stb_image/stb_image.h>

#include <random>

namespace XYZ {

	static glm::vec2 MouseToWorld(const glm::vec2& point, const glm::vec2& windowSize)
	{
		glm::vec2 offset = { windowSize.x / 2,windowSize.y / 2 };
		return { point.x - offset.x, offset.y - point.y };
	}

	static bool Collide(const glm::vec2& pos, const glm::vec2& size, const glm::vec2& point)
	{
		return (pos.x + size.x > point.x &&
			pos.x		   < point.x&&
			pos.y + size.y >  point.y &&
			pos.y < point.y);
	}
	static bool HasExtension(const std::string& path, const char* extension)
	{
		auto lastDot = path.rfind('.');
		auto count = path.size() - lastDot;

		std::string_view view(path.c_str() + lastDot + 1, count);

		if (!view.compare(0, view.size() - 1, extension))
			return true;
		return false;
	}
	
	
	EditorLayer::EditorLayer()
		:
		m_AssetManager("Assets"),
		m_SceneHierarchyPanel(PanelID::SceneHierarchyPanel),
		m_InspectorPanel(PanelID::InspectorPanel),
		m_ScenePanel(PanelID::ScenePanel),
		m_SkinningEditorPanel(PanelID::SkinningEditorPanel)
	{		
	}

	EditorLayer::~EditorLayer()
	{
	}

	void EditorLayer::OnAttach()
	{
		Renderer::Init();
		m_Scene = m_AssetManager.GetAsset<Scene>("Assets/Scenes/scene.xyz")->GetHandle();

		ScriptEngine::Init("Assets/Scripts/XYZScriptExample.dll");
		ScriptEngine::SetSceneContext(m_Scene);

		uint32_t windowWidth = Application::Get().GetWindow().GetWidth();
		uint32_t windowHeight = Application::Get().GetWindow().GetHeight();
		SceneRenderer::SetViewportSize(windowWidth, windowHeight);
		m_Scene->SetViewportSize(windowWidth, windowHeight);
		

		m_Material = m_AssetManager.GetAsset<Material>("Assets/Materials/material.mat")->GetHandle();
		m_Material->SetFlags(XYZ::RenderFlags::TransparentFlag);

		m_TestEntity = m_Scene->GetEntity(2);
		m_SpriteRenderer = &m_TestEntity.GetComponent<SpriteRenderer>();
		m_Transform = &m_TestEntity.GetComponent<TransformComponent>();
		m_TestEntity.AddComponent<ScriptComponent>(ScriptComponent("Example.Script"));
		ScriptEngine::InitScriptEntity(m_TestEntity);
		ScriptEngine::InstantiateEntityClass(m_TestEntity);


		auto test = m_Scene->GetEntity(1);
		test.AddComponent<ScriptComponent>(ScriptComponent("Example.Script"));
		ScriptEngine::InitScriptEntity(test);
		ScriptEngine::InstantiateEntityClass(test);

		m_NewEntity = m_Scene->GetEntity(3);
		
		m_CharacterTexture = Texture2D::Create({ TextureWrap::Clamp, TextureParam::Nearest, TextureParam::Nearest }, "Assets/Textures/player_sprite.png");
		m_CharacterSubTexture = Ref<SubTexture>::Create(m_CharacterTexture, glm::vec2(0, 0), glm::vec2(m_CharacterTexture->GetWidth() / 8, m_CharacterTexture->GetHeight() / 3));
		m_CharacterSubTexture2 = Ref<SubTexture>::Create(m_CharacterTexture, glm::vec2(1, 2), glm::vec2(m_CharacterTexture->GetWidth() / 8, m_CharacterTexture->GetHeight() / 3));
		m_CharacterSubTexture3 = Ref<SubTexture>::Create(m_CharacterTexture, glm::vec2(2, 2), glm::vec2(m_CharacterTexture->GetWidth() / 8, m_CharacterTexture->GetHeight() / 3));

		auto backgroundTexture = m_AssetManager.GetAsset<Texture2D>("Assets/Textures/Backgroundfield.png");
		///////////////////////////////////////////////////////////

		uint32_t count = 5000;
		auto computeMat = Ref<Material>::Create(Shader::Create("Assets/Shaders/Particle/ParticleComputeShader.glsl"));
		auto renderMat = Ref<Material>::Create(Shader::Create("Assets/Shaders/Particle/ParticleShader.glsl"));

		renderMat->Set("u_Texture", Texture2D::Create({TextureWrap::Clamp, TextureParam::Nearest, TextureParam::Nearest}, "Assets/Textures/flame.png"), 0);
		renderMat->Set("u_Texture", Texture2D::Create({TextureWrap::Clamp, TextureParam::Nearest, TextureParam::Nearest}, "Assets/Textures/flame_emission.png"), 1);


		m_Particle = &m_TestEntity.AddComponent<ParticleComponent>(ParticleComponent());
		m_Particle->RenderMaterial = Ref<MaterialInstance>::Create(renderMat);
		m_Particle->ComputeMaterial = Ref<MaterialInstance>::Create(computeMat);
		m_Particle->ParticleEffect = Ref<ParticleEffect>::Create(ParticleEffectConfiguration(count, 2.0f), ParticleLayoutConfiguration());

		m_Particle->ComputeMaterial->Set("u_Speed", 0.2f);
		m_Particle->ComputeMaterial->Set("u_Gravity", 0.0f);
		m_Particle->ComputeMaterial->Set("u_Loop", (int)1);
		m_Particle->ComputeMaterial->Set("u_NumberRows", 8.0f);
		m_Particle->ComputeMaterial->Set("u_NumberColumns", 8.0f);
		m_Particle->ComputeMaterial->Set("u_ParticlesInExistence", 0.0f);
		m_Particle->ComputeMaterial->Set("u_Time", 0.0f);


		std::random_device rd;
		std::mt19937 rng(rd());
		std::uniform_real_distribution<> dist(-1, 1);

		m_Vertices = new XYZ::ParticleVertex[count];
		m_Data = new XYZ::ParticleInformation[count];

		for (int i = 0; i < count; ++i)
		{
			m_Vertices[i].Position = glm::vec4(0.008f * i, 0.0f, 0.0f, 0.0f);
			m_Vertices[i].Color = glm::vec4(0.3, 0.5, 1.0, 1);
			m_Vertices[i].Rotation = 0.0f;
			m_Vertices[i].TexCoordOffset = glm::vec2(0);

			m_Data[i].DefaultPosition = m_Vertices[i].Position;
			
			m_Data[i].ColorBegin = m_Vertices[i].Color;
			m_Data[i].ColorEnd = m_Vertices[i].Color;
			m_Data[i].SizeBegin = 1.0f;
			m_Data[i].SizeEnd = 0.1f;
			m_Data[i].Rotation = dist(rng) * 15;
			m_Data[i].Velocity = glm::vec2(0.0f, fabs(dist(rng)));
			m_Data[i].DefaultVelocity = m_Data[i].Velocity;
			m_Data[i].LifeTime = fabs(dist(rng)) + 4;
		}
		m_Particle->ParticleEffect->SetParticlesRange(m_Vertices, m_Data, 0, count);
		///////////////////////////////////////////////////////////

		Renderer::WaitAndRender();
		
		Ref<RenderTexture> renderTexture = RenderTexture::Create(SceneRenderer::GetFinalRenderPass()->GetSpecification().TargetFramebuffer);
		Ref<SubTexture> renderSubTexture = Ref<SubTexture>::Create(renderTexture, glm::vec4(0.0f, 0.0f, 1.0f, 1.0f));
		Ref<Texture> robotTexture = Texture2D::Create({}, "Assets/Textures/Robot.png");
		Ref<SubTexture> robotSubTexture = Ref<SubTexture>::Create(robotTexture, glm::vec4(0.0f, 0.0f, 1.0f, 1.0f));

		m_SceneHierarchyPanel.SetContext(m_Scene);
		m_ScenePanel.SetContext(m_Scene);
		m_ScenePanel.SetSubTexture(renderSubTexture);
		m_SkinningEditorPanel.SetContext(robotSubTexture);
	}	



	void EditorLayer::OnDetach()
	{
		Renderer::Shutdown();
		m_AssetManager.Serialize();
	}
	void EditorLayer::OnUpdate(Timestep ts)
	{
		Renderer::Clear();
		Renderer::SetClearColor({ 0.1f,0.1f,0.1f,0.1f });
		m_ScenePanel.OnUpdate(ts);
		m_SkinningEditorPanel.OnUpdate(ts);

		if (m_Scene->GetState() == SceneState::Play)
		{
			m_Scene->OnUpdate(ts);
			m_Scene->OnRender();
		}
		else
		{
			m_Scene->OnRenderEditor(m_ScenePanel.GetEditorCamera());
		}

		if ((uint32_t)m_Scene->GetSelectedEntity() != (uint32_t)m_SelectedEntity)
		{
			m_SelectedEntity = m_Scene->GetSelectedEntity();
			m_InspectorPanel.SetContext(m_SelectedEntity);
		}

		if (m_TestEntity.HasComponent<RigidBody2DComponent>())
		{
			float speed = 0.03;
			auto body = m_TestEntity.GetComponent<RigidBody2DComponent>().Body;
			if (body)
			{
				if (Input::IsKeyPressed(KeyCode::KEY_LEFT))
					body->AddVelocity({ -speed,0.0f });
				if (Input::IsKeyPressed(KeyCode::KEY_RIGHT))
					body->AddVelocity({ speed,0.0f });
				if (Input::IsKeyPressed(KeyCode::KEY_UP))
					body->AddVelocity({ 0.0f, speed });
				if (Input::IsKeyPressed(KeyCode::KEY_DOWN))
					body->AddVelocity({ 0.0f, -speed });
			}
		}
		if (m_NewEntity.HasComponent<RigidBody2DComponent>())
		{			
			float speed = 0.06f;
			auto body = m_NewEntity.GetComponent<RigidBody2DComponent>().Body;
			if (body)
			{
				if (Input::IsKeyPressed(KeyCode::KEY_A))
					body->AddVelocity({ -speed,0.0f });
				if (Input::IsKeyPressed(KeyCode::KEY_D))
					body->AddVelocity({ speed,0.0f });
				if (Input::IsKeyPressed(KeyCode::KEY_W))
					body->AddVelocity({ 0.0f, speed });
				if (Input::IsKeyPressed(KeyCode::KEY_S))
					body->AddVelocity({ 0.0f, -speed });
			}
		}
	}

	void EditorLayer::OnEvent(Event& event)
	{			
		EventDispatcher dispatcher(event);
		dispatcher.Dispatch<MouseButtonPressEvent>(Hook(&EditorLayer::onMouseButtonPress, this));
		dispatcher.Dispatch<MouseButtonReleaseEvent>(Hook(&EditorLayer::onMouseButtonRelease, this));	
		dispatcher.Dispatch<WindowResizeEvent>(Hook(&EditorLayer::onWindowResize, this));
		dispatcher.Dispatch<KeyPressedEvent>(Hook(&EditorLayer::onKeyPress, this));
		m_SceneHierarchyPanel.OnEvent(event);
		m_ScenePanel.OnEvent(event);
		m_SkinningEditorPanel.OnEvent(event);
	}

	void EditorLayer::OnInGuiRender()
	{
		m_SceneHierarchyPanel.OnInGuiRender();
		m_InspectorPanel.OnInGuiRender();
		m_ScenePanel.OnInGuiRender();
		m_SkinningEditorPanel.OnInGuiRender();
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