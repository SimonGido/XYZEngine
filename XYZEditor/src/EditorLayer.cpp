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
		m_SceneHierarchy("Layouts/SceneHierarchy.bui"),
		m_Inspector("Layouts/Inspector.bui"),
		m_Main("Layouts/Main.bui"),
		m_SkinningEditor("Layouts/SkinningEditor.bui"),
		m_AnimationEditor("Layouts/AnimationEditor.bui")
	{			
	}

	EditorLayer::~EditorLayer()
	{
	}

	void EditorLayer::OnAttach()
	{
		m_Scene = AssetManager::GetAsset<Scene>(AssetManager::GetAssetHandle("Assets/Scenes/scene.xyz"));
		Scene::ActiveScene = m_Scene;
		m_TestEntity = m_Scene->GetEntityByName("TestEntity");
		

		ScriptEngine::Init("Assets/Scripts/XYZScriptExample.dll");
		ScriptEngine::SetSceneContext(m_Scene);

		uint32_t windowWidth = Application::Get().GetWindow().GetWidth();
		uint32_t windowHeight = Application::Get().GetWindow().GetHeight();
		SceneRenderer::SetViewportSize(windowWidth, windowHeight);

		m_Scene->SetViewportSize(windowWidth, windowHeight);		
		m_EditorCamera = Editor::EditorCamera(30.0f, 1.778f, 0.1f, 1000.0f);
		m_EditorCamera.SetViewportSize((float)windowWidth, (float)windowHeight);
		m_SceneHierarchy.SetContext(m_Scene);

		Renderer::WaitAndRender();

		Ref<RenderTexture> renderTexture = RenderTexture::Create(SceneRenderer::GetFinalRenderPass()->GetSpecification().TargetFramebuffer);
		Ref<SubTexture> renderSubTexture = Ref<SubTexture>::Create(renderTexture, glm::vec2(0.0f, 0.0f));
		Ref<Texture> robotTexture = Texture2D::Create({}, "Assets/Textures/full_simple_char.png");
		Ref<SubTexture> robotSubTexture = Ref<SubTexture>::Create(robotTexture, glm::vec4(0.0f, 0.0f, 1.0f, 1.0f));
		m_SkinningEditor.SetContext(robotSubTexture);

		Ref<Animation> animation = Ref<Animation>::Create(m_TestEntity);
		m_AnimationEditor.SetContext(animation);


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
		AssetSerializer::SerializeAsset(m_Scene);		
	}
	void EditorLayer::OnUpdate(Timestep ts)
	{
		Renderer::Clear();
		Renderer::SetClearColor({ 0.1f,0.1f,0.1f,0.1f });
		
		m_SceneHierarchy.OnUpdate(ts);
		m_Inspector.OnUpdate(ts);
		m_Main.OnUpdate(ts);
		m_SkinningEditor.OnUpdate(ts);		
		m_EditorCamera.OnUpdate(ts);
		m_AnimationEditor.OnUpdate(ts);

		m_Inspector.SetContext(m_Scene->GetSelectedEntity());
		if (m_Scene->GetState() == SceneState::Play)
		{
			m_Scene->OnUpdate(ts);
			m_Scene->OnRender();
		}
		else
		{
			m_Scene->OnRenderEditor(m_EditorCamera);
		}
	}

	void EditorLayer::OnEvent(Event& event)
	{			
		EventDispatcher dispatcher(event);

		dispatcher.Dispatch<MouseButtonPressEvent>(Hook(&EditorLayer::onMouseButtonPress, this));
		dispatcher.Dispatch<MouseButtonReleaseEvent>(Hook(&EditorLayer::onMouseButtonRelease, this));	
		dispatcher.Dispatch<WindowResizeEvent>(Hook(&EditorLayer::onWindowResize, this));
		dispatcher.Dispatch<KeyPressedEvent>(Hook(&EditorLayer::onKeyPress, this));
		

		m_SceneHierarchy.OnEvent(event);
		m_SkinningEditor.OnEvent(event);
		m_Main.OnEvent(event);
		m_AnimationEditor.OnEvent(event);
		m_EditorCamera.OnEvent(event);
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
		SceneRenderer::SetViewportSize((uint32_t)event.GetWidth(), (uint32_t)event.GetHeight());
		m_EditorCamera.SetViewportSize((float)event.GetWidth(), (float)event.GetHeight());
		if (m_Scene.Raw())
			m_Scene->SetViewportSize((uint32_t)event.GetWidth(), (uint32_t)event.GetHeight());
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