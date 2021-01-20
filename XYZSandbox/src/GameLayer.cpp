#include "GameLayer.h"


namespace XYZ {

	static LuaApp* s_LuaApp = nullptr;

	GameLayer::GameLayer()
	{
	}

	GameLayer::~GameLayer()
	{
	}

	void GameLayer::OnAttach()
	{
		Renderer::Init();

		m_Scene = Ref<Scene>::Create("Scene");
		MonoScriptEngine::Init("XYZScriptCore.dll");

		m_EditorCamera = EditorCamera(30.0f, 1.778f, 0.1f, 1000.0f);
		auto& app = Application::Get();
		m_EditorCamera.SetViewportSize((float)app.GetWindow().GetWidth(), (float)app.GetWindow().GetHeight());
		SceneRenderer::SetViewportSize(app.GetWindow().GetWidth(), app.GetWindow().GetHeight());

		Ref<Material> material = Ref<Material>::Create(Shader::Create("Assets/Shaders/DefaultShader.glsl"));
		Ref<Texture2D> texture = Texture2D::Create({ TextureWrap::Clamp, TextureParam::Nearest, TextureParam::Nearest }, "Assets/Textures/player_sprite.png");
		Ref<SubTexture> subTexture = Ref<SubTexture>::Create(texture, glm::vec2(0, 0), glm::vec2(texture->GetWidth() / 8, texture->GetHeight() / 3));


		
		material->Set("u_Color", glm::vec4(1.0f));
		m_Entity = m_Scene->CreateEntity("Test Entity", GUID());
		
		SpriteRenderer& spriteRenderer = m_Entity.AddComponent<SpriteRenderer>(SpriteRenderer(
			material, subTexture, glm::vec4(1.0f), 0
		));


		Ref<Animation> walkRightAnimation = Ref<Animation>::Create(2.0f);
		{
			Ref<SubTexture> firstFrame = Ref<SubTexture>::Create(texture, glm::vec2(0, 1), glm::vec2(texture->GetWidth() / 8, texture->GetHeight() / 3));
			Ref<SubTexture> secondFrame = Ref<SubTexture>::Create(texture, glm::vec2(1, 1), glm::vec2(texture->GetWidth() / 8, texture->GetHeight() / 3));
			auto property = walkRightAnimation->AddProperty<Ref<SubTexture>>(spriteRenderer.SubTexture);
			property->KeyFrames.push_back({ firstFrame, 1.0f });
			property->KeyFrames.push_back({ secondFrame, 2.0f });
		}

		Ref<Animation> walkLeftAnimation = Ref<Animation>::Create(2.0f);
		{
			Ref<SubTexture> firstFrame = Ref<SubTexture>::Create(texture, glm::vec2(2, 1), glm::vec2(texture->GetWidth() / 8, texture->GetHeight() / 3));
			Ref<SubTexture> secondFrame = Ref<SubTexture>::Create(texture, glm::vec2(3, 1), glm::vec2(texture->GetWidth() / 8, texture->GetHeight() / 3));
			auto property = walkLeftAnimation->AddProperty<Ref<SubTexture>>(spriteRenderer.SubTexture);
			property->KeyFrames.push_back({ firstFrame, 1.0f });
			property->KeyFrames.push_back({ secondFrame, 2.0f });
		}


		AnimatorComponent& animator = m_Entity.AddComponent<AnimatorComponent>(AnimatorComponent());
		uint32_t walkRight = animator.Controller.AddAnimation(walkRightAnimation);
		uint32_t walkLeft = animator.Controller.AddAnimation(walkLeftAnimation);
		
		StateMachine<32>& stateMachine = animator.Controller.GetStateMachine();
		stateMachine.GetState(walkRight).AllowTransition(walkLeft);
		stateMachine.GetState(walkLeft).AllowTransition(walkRight);

		auto specs = SceneRenderer::GetFinalRenderPass()->GetSpecification().TargetFramebuffer->GetSpecification();
		specs.SwapChainTarget = true;
		SceneRenderer::GetFinalRenderPass()->GetSpecification().TargetFramebuffer->SetSpecification(specs);
	
		LuaEntity::SetActiveScene(m_Scene);
		s_LuaApp = new LuaApp("Assets/Scripts", "Test.lua");
	}

	void GameLayer::OnDetach()
	{
		Renderer::Shutdown();
	}

	void GameLayer::OnUpdate(Timestep ts)
	{
		Renderer::Clear();
		Renderer::SetClearColor({ 0.1f,0.1f,0.1f,0.1f });

		m_EditorCamera.OnUpdate(ts);
		m_Scene->OnUpdate(ts);
		m_Scene->OnRenderEditor(m_EditorCamera);
		s_LuaApp->OnUpdate(ts);
	}

	void GameLayer::OnEvent(Event& event)
	{
		EventDispatcher dispatcher(event);
		dispatcher.Dispatch<WindowResizeEvent>(Hook(&GameLayer::onWindowResize, this));
		m_EditorCamera.OnEvent(event);
	}

	void GameLayer::OnInGuiRender()
	{
		if (InGui::Begin(1, "Test Window hahahahahaha", glm::vec2(200.0f), glm::vec2(100.0f)))
		{

		}
		else
		{
		}
		InGui::End();
		if (InGui::Begin(0, "Test Window", glm::vec2(0.0f), glm::vec2(500.0f)))
		{
			InGui::Button("Buttonik", glm::vec2(50.0f,50.0f));
			InGui::Button("Buttonik", glm::vec2(50.0f, 50.0f));
			InGui::Button("Buttonik", glm::vec2(50.0f, 50.0f));
			InGui::Button("Buttonik", glm::vec2(50.0f, 50.0f));
			InGui::Button("Buttonik", glm::vec2(50.0f, 50.0f));
		}
		else
		{
		}
		InGui::End();		
	}

	bool GameLayer::onWindowResize(WindowResizeEvent& event)
	{
		m_Scene->SetViewportSize((uint32_t)event.GetWidth(), (uint32_t)event.GetHeight());
		m_EditorCamera.SetViewportSize((float)event.GetWidth(), (float)event.GetHeight());
		return false;
	}
}