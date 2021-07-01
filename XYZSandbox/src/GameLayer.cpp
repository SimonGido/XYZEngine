#include "GameLayer.h"


namespace XYZ {

	static LuaModule* s_LuaModule= nullptr;

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
		ScriptEngine::Init("Assets/Scripts/XYZScriptExample.dll");
		ScriptEngine::SetSceneContext(m_Scene);

		m_EditorCamera = Editor::EditorCamera(30.0f, 1.778f, 0.1f, 1000.0f);
		auto& app = Application::Get();
		m_EditorCamera.SetViewportSize((float)app.GetWindow().GetWidth(), (float)app.GetWindow().GetHeight());
		SceneRenderer::SetViewportSize(app.GetWindow().GetWidth(), app.GetWindow().GetHeight());

		Ref<Material> material = Ref<Material>::Create(Shader::Create("Assets/Shaders/DefaultShader.glsl"));
		Ref<Texture2D> texture = Texture2D::Create({ TextureWrap::Clamp, TextureParam::Nearest, TextureParam::Nearest }, "Assets/Textures/player_sprite.png");
	
		
		material->Set("u_Color", glm::vec4(1.0f));
		m_Entity = m_Scene->CreateEntity("Test Entity", GUID());
		
		m_Entity.AddComponent<ScriptComponent>(ScriptComponent("Example.Script"));
		ScriptEngine::InitScriptEntity(m_Entity);
		ScriptEngine::InstantiateEntityClass(m_Entity);

		
		auto specs = SceneRenderer::GetFinalRenderPass()->GetSpecification().TargetFramebuffer->GetSpecification();
		specs.SwapChainTarget = true;
		SceneRenderer::GetFinalRenderPass()->GetSpecification().TargetFramebuffer->SetSpecification(specs);
	
		LuaEntity::SetActiveScene(m_Scene);
		s_LuaModule = new LuaModule("Assets/Lua", "Test.lua");
	}

	void GameLayer::OnDetach()
	{
		ScriptEngine::Shutdown();
		Renderer::Shutdown();
	}

	void GameLayer::OnUpdate(Timestep ts)
	{
		Renderer::Clear();
		Renderer::SetClearColor({ 0.1f,0.1f,0.1f,0.1f });

		m_EditorCamera.OnUpdate(ts);
		m_Scene->OnUpdate(ts);
		m_Scene->OnRenderEditor(m_EditorCamera, ts);
		s_LuaModule->OnUpdate(ts);
	}

	void GameLayer::OnEvent(Event& event)
	{
		EventDispatcher dispatcher(event);
		dispatcher.Dispatch<WindowResizeEvent>(Hook(&GameLayer::onWindowResize, this));
		m_EditorCamera.OnEvent(event);
	}

	bool GameLayer::onWindowResize(WindowResizeEvent& event)
	{
		m_Scene->SetViewportSize((uint32_t)event.GetWidth(), (uint32_t)event.GetHeight());
		m_EditorCamera.SetViewportSize((float)event.GetWidth(), (float)event.GetHeight());
		return false;
	}
}