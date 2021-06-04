#include "EditorLayer.h"

#include <glm/glm.hpp>
#include <glm/gtx/transform.hpp>

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
	{			
	}

	EditorLayer::~EditorLayer()
	{
	}

	void EditorLayer::OnAttach()
	{
		ScriptEngine::Init("Assets/Scripts/XYZScriptExample.dll");
		m_Scene = AssetManager::GetAsset<Scene>(AssetManager::GetAssetHandle("Assets/Scenes/scene.xyz"));
		m_SceneHierarchy.SetContext(m_Scene);
		m_ScenePanel.SetContext(m_Scene);

		m_TestEntity = m_Scene->GetEntityByName("TestEntity");
		

		
		ScriptEngine::SetSceneContext(m_Scene);

		uint32_t windowWidth = Application::Get().GetWindow().GetWidth();
		uint32_t windowHeight = Application::Get().GetWindow().GetHeight();
		SceneRenderer::SetViewportSize(windowWidth, windowHeight);
		m_Scene->SetViewportSize(windowWidth, windowHeight);		

		Renderer::WaitAndRender();

		Ref<Texture> robotTexture = Texture2D::Create({}, "Assets/Textures/full_simple_char.png");
		Ref<SubTexture> robotSubTexture = Ref<SubTexture>::Create(robotTexture, glm::vec4(0.0f, 0.0f, 1.0f, 1.0f));
		m_Test = robotSubTexture;

		Ref<Animation> animation = Ref<Animation>::Create(m_TestEntity);
		

		animation->CreateTrack<TransformTrack>();
		auto track = animation->FindTrack<TransformTrack>();
		if (track)
		{
			{
				KeyFrame<glm::vec3> key;
				key.Value = glm::vec3(0.0f, 0.0f, 0.0f);
				key.EndTime = 0.0f;
				track->AddKeyFrame<glm::vec3>(key, TransformTrack::PropertyType::Translation);
			}
			{
				KeyFrame<glm::vec3> key;
				key.Value = glm::vec3(0.0f, 0.0f, 7.0f);
				key.EndTime = 2.0f;
				track->AddKeyFrame<glm::vec3>(key, TransformTrack::PropertyType::Rotation);
			}
			{
				KeyFrame<glm::vec3> key;
				key.Value = glm::vec3(0.0f, 2.0f, 0.0f);
				key.EndTime = 4.0f;
				track->AddKeyFrame<glm::vec3>(key, TransformTrack::PropertyType::Translation);
			}
			{
				KeyFrame<glm::vec3> key;
				key.Value = glm::vec3(0.0f, 0.0f, 0.0f);
				key.EndTime = 6.0f;
				track->AddKeyFrame<glm::vec3>(key, TransformTrack::PropertyType::Translation);
			}
			{
				KeyFrame<glm::vec3> key;
				key.Value = glm::vec3(0.0f, 0.0f, 0.0f);
				key.EndTime = 6.0f;
				track->AddKeyFrame<glm::vec3>(key, TransformTrack::PropertyType::Rotation);
			}
		}
		animation->UpdateLength();
		AnimatorComponent& animator = m_TestEntity.EmplaceComponent<AnimatorComponent>();
		animator.Animation = animation;
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
		m_Inspector.SetContext(m_Scene->GetSelectedEntity());
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