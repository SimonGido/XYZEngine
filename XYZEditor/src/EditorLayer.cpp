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
		m_SceneHierarchyPanel(PanelID::SceneHierarchyPanel),
		m_ScenePanel(PanelID::ScenePanel),
		m_InspectorPanel(PanelID::InspectorPanel),
		m_SkinningEditorPanel(PanelID::SkinningEditorPanel)
	{		
	}

	EditorLayer::~EditorLayer()
	{
	}

	void EditorLayer::OnAttach()
	{
		AssetManager::Init();
		Renderer::Init();

		auto shader = AssetManager::GetAsset<Shader>(AssetManager::GetAssetHandle("Assets/Shaders/DefaultShader.glsl.shader"));
		auto texture = AssetManager::GetAsset<Texture2D>(AssetManager::GetAssetHandle("Assets/Textures/player_sprite.png.tex"));
		auto subTexture = AssetManager::GetAsset<SubTexture>(AssetManager::GetAssetHandle("Assets/SubTextures/player.subtex"));
		auto material = AssetManager::GetAsset<Material>(AssetManager::GetAssetHandle("Assets/Materials/Material.mat"));
		m_Scene = AssetManager::GetAsset<Scene>(AssetManager::GetAssetHandle("Assets/Scenes/scene.xyz"));
		m_TestEntity = m_Scene->GetEntityByName("TestEntity");
		
		m_TestEntity.EmplaceComponent<ScriptComponent>("Example.Script");


		ScriptEngine::Init("Assets/Scripts/XYZScriptExample.dll");
		ScriptEngine::SetSceneContext(m_Scene);

		uint32_t windowWidth = Application::Get().GetWindow().GetWidth();
		uint32_t windowHeight = Application::Get().GetWindow().GetHeight();
		SceneRenderer::SetViewportSize(windowWidth, windowHeight);
		m_Scene->SetViewportSize(windowWidth, windowHeight);
		


		ScriptEngine::InitScriptEntity(m_TestEntity);
		ScriptEngine::InstantiateEntityClass(m_TestEntity);


		Renderer::WaitAndRender();
		
		Ref<RenderTexture> renderTexture = RenderTexture::Create(SceneRenderer::GetFinalRenderPass()->GetSpecification().TargetFramebuffer);
		Ref<SubTexture> renderSubTexture = Ref<SubTexture>::Create(renderTexture, glm::vec4(0.0f, 0.0f, 1.0f, 1.0f));
		Ref<Texture> robotTexture = Texture2D::Create({}, "Assets/Textures/Robot.png");
		Ref<SubTexture> robotSubTexture = Ref<SubTexture>::Create(robotTexture, glm::vec4(0.0f, 0.0f, 1.0f, 1.0f));

		m_SceneHierarchyPanel.SetContext(m_Scene);
		m_ScenePanel.SetContext(m_Scene);
		m_ScenePanel.SetSubTexture(renderSubTexture);
		m_SkinningEditorPanel.SetContext(robotSubTexture);


		std::initializer_list<IGHierarchy> types{{
			IGElementType::Window,
			{
				IGElementType::Checkbox,
				IGElementType::Checkbox,
				IGElementType::Checkbox,
				IGElementType::Checkbox,
				IGElementType::Checkbox,
				IGElementType::Checkbox,
				IGElementType::Checkbox,
			}
		}};
		m_HandleCount = IG::AllocateUI(
			types, 
			&m_Handles
		).second;
	}	



	void EditorLayer::OnDetach()
	{
		AssetSerializer::SerializeAsset(m_Scene);
		Renderer::Shutdown();
		AssetManager::Shutdown();
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
		//m_SceneHierarchyPanel.OnInGuiRender();
		//m_InspectorPanel.OnInGuiRender();
		//m_ScenePanel.OnInGuiRender();
		//m_SkinningEditorPanel.OnInGuiRender();

		IG::BeginUI(0);
	
		IG::UI<IGWindow>(m_Handles[0], "Test 1");

		bool checked = false;
		IG::UI<IGCheckbox>(m_Handles[1], "Label", checked);
		IG::UI<IGCheckbox>(m_Handles[2], "Label", checked);
		IG::UI<IGCheckbox>(m_Handles[3], "Label", checked);
		IG::UI<IGCheckbox>(m_Handles[4], "Label", checked);
		IG::UI<IGCheckbox>(m_Handles[5], "Label", checked);
		IG::UI<IGCheckbox>(m_Handles[6], "Label", checked);
		IG::UI<IGCheckbox>(m_Handles[7], "Label", checked);
		IG::EndUI();
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