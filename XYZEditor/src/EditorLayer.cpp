#include "EditorLayer.h"

#include "Panels/Panel.h"

#include <glm/glm.hpp>
#include <glm/gtx/transform.hpp>

#include <stb_image/stb_image.h>

namespace XYZ {

	static glm::vec2 MouseToWorld(const glm::vec2& point, const glm::vec2& windowSize)
	{
		glm::vec2 offset = { windowSize.x / 2,windowSize.y / 2 };
		return { point.x - offset.x, offset.y - point.y };
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
	static bool Collide(const glm::vec2& pos, const glm::vec2& size, const glm::vec2& point)
	{
		return (pos.x + size.x > point.x &&
			pos.x		   < point.x&&
			pos.y + size.y >  point.y &&
			pos.y < point.y);
	}
	
	EditorLayer::EditorLayer()
		:
		m_SpriteEditorPanel(m_AssetManager),
		m_NodeGraph(false)
	{
	}

	EditorLayer::~EditorLayer()
	{
	}

	void EditorLayer::OnAttach()
	{
		int width, height, channels;
		stbi_set_flip_vertically_on_load(1);
		uint8_t* pixels = (uint8_t*)stbi_load("Assets/Textures/Gui/Prohibited.png", &width, &height, &channels, 0);
		m_ProhibitedCursor = Application::Get().GetWindow().CreateCustomCursor(pixels, width, height, width / 2.0f, height / 2.0f);
		

		Renderer::Init();
		NativeScriptEngine::Init();

		NativeScriptEngine::SetOnReloadCallback([this] {
			auto storage = m_Scene->GetECS().GetComponentStorage<NativeScriptComponent>();
			for (int i = 0; i < storage->Size(); ++i)
			{
				(*storage)[i].ScriptableEntity = (ScriptableEntity*)NativeScriptEngine::CreateScriptObject((*storage)[i].ScriptObjectName);
				if ((*storage)[i].ScriptableEntity)
				{
					(*storage)[i].ScriptableEntity->Entity = m_StoredEntitiesWithScript[i];
					(*storage)[i].ScriptableEntity->OnCreate();
				}
			}
		});

		NativeScriptEngine::SetOnRecompileCallback([this]() {		
			auto storage = m_Scene->GetECS().GetComponentStorage<NativeScriptComponent>();
			for (int i = 0; i < storage->Size(); ++i)
			{
				if ((*storage)[i].ScriptableEntity)
				{
					m_StoredEntitiesWithScript.push_back((*storage)[i].ScriptableEntity->Entity);
				}
			}
		});
		

		uint32_t windowWidth = Application::Get().GetWindow().GetWidth();
		uint32_t windowHeight = Application::Get().GetWindow().GetHeight();
		m_Scene = m_AssetManager.GetAsset<Scene>("Assets/Scenes/scene.xyz");
		m_Scene->SetViewportSize(windowWidth, windowHeight);
		SceneManager::Get().SetActive(m_Scene);

		m_Material = m_AssetManager.GetAsset<Material>("Assets/Materials/material.mat");
		m_Material->SetFlags(XYZ::RenderFlags::TransparentFlag);

		m_TestEntity = m_Scene->GetEntity(2);
		m_SpriteRenderer = m_TestEntity.GetComponent<SpriteRenderer>();
		m_Transform = m_TestEntity.GetComponent<TransformComponent>();
		m_Animator = m_TestEntity.EmplaceComponent<AnimatorComponent>();

		m_CharacterTexture = Texture2D::Create(XYZ::TextureWrap::Clamp, TextureParam::Nearest, TextureParam::Nearest, "Assets/Textures/player_sprite.png");
		m_CharacterSubTexture = Ref<SubTexture2D>::Create(m_CharacterTexture, glm::vec2(0, 0), glm::vec2(m_CharacterTexture->GetWidth() / 8, m_CharacterTexture->GetHeight() / 3));
		m_CharacterSubTexture2 = Ref<SubTexture2D>::Create(m_CharacterTexture, glm::vec2(1, 2), glm::vec2(m_CharacterTexture->GetWidth() / 8, m_CharacterTexture->GetHeight() / 3));
		m_CharacterSubTexture3 = Ref<SubTexture2D>::Create(m_CharacterTexture, glm::vec2(2, 2), glm::vec2(m_CharacterTexture->GetWidth() / 8, m_CharacterTexture->GetHeight() / 3));


		
		m_IdleAnimation = Ref<Animation>::Create(3.0f);
		auto prop = m_IdleAnimation->AddProperty<glm::vec4>(m_SpriteRenderer->Color);
		auto posProperty = m_IdleAnimation->AddProperty<glm::vec3>(m_Position);
		auto rotProperty = m_IdleAnimation->AddProperty<glm::vec3>(m_Rotation);
		auto spriteProperty = m_IdleAnimation->AddProperty<Ref<SubTexture2D>>(m_SpriteRenderer->SubTexture);

		prop->KeyFrames.push_back({ {1,0,0,1},0.0f });
		prop->KeyFrames.push_back({ {0,1,0,1},1.0f });
		prop->KeyFrames.push_back({ {0,0,1,1},2.0f });
		prop->KeyFrames.push_back({ {1,0,0,1},3.0f });


		posProperty->KeyFrames.push_back({ {0,0,0},0.0f });
		posProperty->KeyFrames.push_back({ {-1,0,0},1.0f });
		posProperty->KeyFrames.push_back({ {0,1,0},2.0f });
		posProperty->KeyFrames.push_back({ {0,0,0},3.0f });

		rotProperty->KeyFrames.push_back({ {0,0,0},0.0f });
		rotProperty->KeyFrames.push_back({ {0,0,1.5},1.0f });
		rotProperty->KeyFrames.push_back({ {0,0,2},2.0f });
		rotProperty->KeyFrames.push_back({ {0,0,0},3.0f });

		spriteProperty->KeyFrames.push_back({ m_CharacterSubTexture,0.0f });
		spriteProperty->KeyFrames.push_back({ m_CharacterSubTexture2,1.0f });
		spriteProperty->KeyFrames.push_back({ m_CharacterSubTexture3,2.0f });
		spriteProperty->KeyFrames.push_back({ m_CharacterSubTexture,3.0f });


		m_RunAnimation = Ref<Animation>::Create(3.0f);
		auto posProperty2 = m_RunAnimation->AddProperty<glm::vec3>(m_Position);
		posProperty2->KeyFrames.push_back({ {0,0,0},0.0f });
		posProperty2->KeyFrames.push_back({ {-1,0,0},1.0f });
		posProperty2->KeyFrames.push_back({ {0,1,0},2.0f });
		posProperty2->KeyFrames.push_back({ {0,0,0},3.0f });


		auto spriteProperty2 = m_RunAnimation->AddProperty<Ref<SubTexture2D>>(m_SpriteRenderer->SubTexture);
		spriteProperty2->KeyFrames.push_back({ m_CharacterSubTexture,0.0f });
		spriteProperty2->KeyFrames.push_back({ m_CharacterSubTexture2,1.0f });
		spriteProperty2->KeyFrames.push_back({ m_CharacterSubTexture3,2.0f });
		spriteProperty2->KeyFrames.push_back({ m_CharacterSubTexture,3.0f });

		m_AnimationController = Ref<AnimationController>::Create();
		m_AnimationController->AddAnimation("Idle", m_IdleAnimation);
		m_AnimationController->AddAnimation("Run", m_RunAnimation);
		m_AnimationController->AddAnimation("Test1", m_RunAnimation);
		m_AnimationController->AddAnimation("Test2", m_RunAnimation);
		m_AnimationController->AddAnimation("Test3", m_RunAnimation);
		m_AnimationController->AddAnimation("Test4", m_RunAnimation);
		m_AnimationController->AddAnimation("Test5", m_RunAnimation);
		m_AnimationController->AddAnimation("Test6", m_RunAnimation);

		m_Animator->Controller = m_AnimationController;
		m_Animator->Controller->SetDefaultAnimation("Run");

		m_SceneHierarchyPanel.SetContext(m_Scene);

		
		InGui::RenderWindow(PanelID::Scene, "Scene", 0, { 0,0 }, { 200,200 });
		InGui::End();
		m_SceneWindow = InGui::GetWindow(PanelID::Scene);
		m_SceneWindow->Flags &= ~InGuiWindowFlag::EventListener;
		m_EditorCamera.OnResize(m_SceneWindow->Size);

		InGui::SetUIOffset(10.0f);
		InGui::Begin(PanelID::Test, "Test", { 0,0 }, { 200,200 });
		InGui::End();
		InGui::GetWindow(PanelID::Test)->Flags |= (InGuiWindowFlag::MenuEnabled | InGuiWindowFlag::Visible | InGuiWindowFlag::EventListener);

		InGui::GetWindow(PanelID::Scene)->OnResizeCallback = Hook(&EditorLayer::onResizeSceneWindow, this);
		
		float divisor = 8.0f;
		auto& renderConfig = InGui::GetRenderConfiguration();
		renderConfig.SubTexture[PLAY] = Ref<SubTexture2D>::Create(renderConfig.Texture, glm::vec2(2, 2), glm::vec2(renderConfig.Texture->GetWidth() / divisor, renderConfig.Texture->GetHeight() / divisor));
		renderConfig.SubTexture[PAUSE] = Ref<SubTexture2D>::Create(renderConfig.Texture, glm::vec2(2, 1), glm::vec2(renderConfig.Texture->GetWidth() / divisor, renderConfig.Texture->GetHeight() / divisor));

		auto backgroundTexture = m_AssetManager.GetAsset<Texture2D>("Assets/Textures/Backgroundfield.png");
		m_SpriteEditorPanel.SetContext(backgroundTexture);
		m_AnimatorInspectorLayout.SetContext(m_AnimationController);
		m_AnimatorGraphLayout.SetContext(m_AnimationController);
		m_InspectorPanel.SetInspectorLayout(&m_EntityInspectorLayout);
		m_GraphPanel.SetGraphLayout(&m_AnimatorGraphLayout);
	}

	void EditorLayer::OnDetach()
	{
		NativeScriptEngine::Shutdown();
		Renderer::Shutdown();
	}
	void EditorLayer::OnUpdate(Timestep ts)
	{
		Renderer::SetClearColor(glm::vec4(0.2, 0.2, 0.2, 1));
		Renderer::Clear();
		NativeScriptEngine::Update(ts);

		if (m_ActiveWindow)
		{
			m_EditorCamera.OnUpdate(ts);
		}
		
		if ((uint32_t)m_SelectedEntity != (uint32_t)m_SceneHierarchyPanel.GetSelectedEntity())
		{
			m_SelectedEntity = m_SceneHierarchyPanel.GetSelectedEntity();
			m_EntityInspectorLayout.SetContext(m_SelectedEntity);
			m_Scene->SetSelectedEntity(m_SelectedEntity);
		}
			
		if (m_ScalingEntity)
		{
			auto mousePos = InGui::GetWorldPosition(*InGui::GetWindow(PanelID::Scene), m_EditorCamera.GetPosition(), m_EditorCamera.GetAspectRatio(), m_EditorCamera.GetZoomLevel());

			glm::vec3 scale = m_ModifiedScale;
			scale.x += mousePos.x - m_StartMousePos.x;
			scale.y += mousePos.y - m_StartMousePos.y;
	
			m_ModifiedTransform->Scale = scale;	
			m_EntityInspectorLayout.SetContext(m_SelectedEntity);
		}
		else if (m_MovingEntity)
		{
			auto mousePos = InGui::GetWorldPosition(*InGui::GetWindow(PanelID::Scene), m_EditorCamera.GetPosition(), m_EditorCamera.GetAspectRatio(), m_EditorCamera.GetZoomLevel());

			glm::vec3 translation = m_ModifiedTranslation;
			translation.x +=  (mousePos.x - m_StartMousePos.x);
			translation.y +=  (mousePos.y - m_StartMousePos.y);
			
			m_ModifiedTransform->Translation = translation;
			m_EntityInspectorLayout.SetContext(m_SelectedEntity);
		}
		else if (m_RotatingEntity)
		{
			auto mousePos = InGui::GetWorldPosition(*InGui::GetWindow(PanelID::Scene), m_EditorCamera.GetPosition(), m_EditorCamera.GetAspectRatio(), m_EditorCamera.GetZoomLevel());

			float rotation = m_ModifiedRotation.x + (mousePos.x - m_StartMousePos.x);
		
			m_ModifiedTransform->Rotation.z = rotation;
			m_EntityInspectorLayout.SetContext(m_SelectedEntity);
		}
	
		m_Scene->OnUpdate(ts);

		if (m_Scene->GetState() == SceneState::Edit)
			m_Scene->OnRenderEditor(m_EditorCamera); 
		else if (m_Scene->GetState() == SceneState::Play)
			m_Scene->OnRender();
	}
	void EditorLayer::OnEvent(Event& event)
	{
		m_EditorCamera.OnEvent(event);
		m_GraphPanel.OnEvent(event);
		m_SpriteEditorPanel.OnEvent(event);
		m_ProjectBrowserPanel.OnEvent(event);

		EventDispatcher dispatcher(event);
		dispatcher.Dispatch<WindowResizeEvent>(Hook(&EditorLayer::onWindowResized, this));
		dispatcher.Dispatch<MouseButtonPressEvent>(Hook(&EditorLayer::onMouseButtonPress, this));
		dispatcher.Dispatch<MouseButtonReleaseEvent>(Hook(&EditorLayer::onMouseButtonRelease, this));
		dispatcher.Dispatch<KeyPressedEvent>(Hook(&EditorLayer::onKeyPress, this));
		dispatcher.Dispatch<KeyReleasedEvent>(Hook(&EditorLayer::onKeyRelease, this));
	}

	void EditorLayer::OnInGuiRender(Timestep ts)
	{
		if (m_Dragging && m_ProjectBrowserPanel.GetSelectedFileIndex() != -1)
		{
			auto& renderConfig = InGui::GetRenderConfiguration();
			auto [mx, my] = Input::GetMousePosition();
			auto [width, height] = Input::GetWindowSize();
			glm::vec2 size = { 25.0f,25.0f };
			glm::vec2 position = MouseToWorld({ mx,my }, { width,height }) - (size / 2.0f);
			
			
			if (m_ProhibitedCursor && !m_EntityInspectorLayout.ValidExtension(m_ProjectBrowserPanel.GetSelectedFilePath()))
			{
				Application::Get().GetWindow().SetCustomCursor(m_ProhibitedCursor);
			}
			else
			{
				Application::Get().GetWindow().SetStandardCursor(WindowCursors::XYZ_ARROW_CURSOR);
			}
		}

		if (m_GraphPanel.OnInGuiRender(ts))
		{
			m_InspectorPanel.SetInspectorLayout(&m_AnimatorInspectorLayout);
		}
		if (m_SpriteEditorPanel.OnInGuiRender(ts))
		{
			m_InspectorPanel.SetInspectorLayout(&m_SpriteEditorInspectorLayout);
		}
		m_SceneHierarchyPanel.OnInGuiRender();
		m_InspectorPanel.OnInGuiRender();
		m_ProjectBrowserPanel.OnInGuiRender();

		if (InGui::RenderWindow(0, "Scene", SceneRenderer::GetFinalColorBufferRendererID(), { 0,0 }, { 200,200 }))
		{
			m_ActiveWindow = true;
			m_InspectorPanel.SetInspectorLayout(&m_EntityInspectorLayout);
			auto& renderConfig = InGui::GetRenderConfiguration();
			
			if (InGui::Icon({}, glm::vec2(40.0f, 40.0f), renderConfig.SubTexture[PLAY], renderConfig.TextureID) & InGuiReturnType::Clicked)
			{
				m_Scene->OnPlay();
				m_Scene->SetState(SceneState::Play);
			}
			if (InGui::Icon({}, glm::vec2(40.0f, 40.0f), renderConfig.SubTexture[PAUSE], renderConfig.TextureID) & InGuiReturnType::Clicked)
			{
				m_Scene->SetState(SceneState::Edit);
			}
			InGui::Selector(m_Selecting);
		}
		else
		{
			m_ActiveWindow = false;
		}
		InGui::End();


		if (InGui::Begin(PanelID::TestPanel, "Test Panel", { 0,0 }, { 200,200 }))
		{
			
		}
		InGui::End();

		if (InGui::Begin(PanelID::Test, "Test", { 0,0 }, { 200,200 }))
		{
			
			InGui::MenuBar("File", 70, m_MenuOpen);
			if (m_MenuOpen)
			{
				if (InGui::MenuItem("Load Scene", { 150,25 }) & InGuiReturnType::Clicked)
				{
					auto& app = Application::Get();
					std::string filepath = app.OpenFile("(*.xyz)\0*.xyz\0");
					if (!filepath.empty())
					{
						m_Scene = m_AssetManager.GetAsset<Scene>(filepath);
					}
					m_MenuOpen = false;
				}
				else if (InGui::MenuItem("Create Script", { 150,25 }) & InGuiReturnType::Clicked)
				{
					auto& app = Application::Get();
					std::string filepath = app.CreateNewFile("(*.cpp)\0*.cpp\0");
					if (!filepath.empty())
					{
						PerModuleInterface::g_pRuntimeObjectSystem->AddToRuntimeFileList(filepath.c_str());
					}
					m_MenuOpen = false;
				}
				else if (InGui::MenuItem("Load Script", { 150,25 }) & InGuiReturnType::Clicked)
				{
					auto& app = Application::Get();
					std::string filepath = app.OpenFile("(*.cpp)\0*.cpp\0");
					if (!filepath.empty())
					{
						PerModuleInterface::g_pRuntimeObjectSystem->AddToRuntimeFileList(filepath.c_str());
					}
					m_MenuOpen = false;
				}

			}
			if (InGui::Button("Compile", { 100,25 }) & InGuiReturnType::Clicked)
			{
				PerModuleInterface::g_pRuntimeObjectSystem->CompileAll(true);
			}
		}
		InGui::End();
	}

	bool EditorLayer::onWindowResized(WindowResizeEvent& event)
	{
		m_Scene->SetViewportSize((uint32_t)(event.GetWidth()), (uint32_t)(event.GetHeight()));
		m_EditorCamera.OnResize(m_SceneWindow->Size);
		return false;
	}
	bool EditorLayer::onMouseButtonPress(MouseButtonPressEvent& event)
	{
		if (event.IsButtonPressed(MouseCode::XYZ_MOUSE_BUTTON_LEFT))
		{
			auto win = InGui::GetWindow(PanelID::Scene);
			auto [mx, my] = Input::GetMousePosition();
			auto [width, height] = Input::GetWindowSize();

			glm::vec2 mousePos = MouseToWorld({ mx,my }, { width,height });
			glm::vec2 relativeMousePos = InGui::GetWorldPosition(*win, m_EditorCamera.GetPosition(), m_EditorCamera.GetAspectRatio(), m_EditorCamera.GetZoomLevel());

			if (Collide(win->Position, win->Size, mousePos))
				m_SceneHierarchyPanel.SelectEntity(relativeMousePos);
		
			m_Dragging = true;
		}
		else if (event.IsButtonPressed(MouseCode::XYZ_MOUSE_BUTTON_RIGHT))
		{
			m_SpriteEditorInspectorLayout.SetContext(m_SpriteEditorPanel.GetSelectedSprite());
		}
		
		return false;
	}
	bool EditorLayer::onMouseButtonRelease(MouseButtonReleaseEvent& event)
	{
		if (event.IsButtonReleased(MouseCode::XYZ_MOUSE_BUTTON_LEFT))
		{
			m_Dragging = false;
			Application::Get().GetWindow().SetStandardCursor(WindowCursors::XYZ_ARROW_CURSOR);
			m_EntityInspectorLayout.AttemptSetAsset(m_ProjectBrowserPanel.GetSelectedFilePath(), m_AssetManager);	
			if (HasExtension(m_ProjectBrowserPanel.GetSelectedFilePath(), "png"))
			{
				auto texture = m_AssetManager.GetAsset<Texture2D>(m_ProjectBrowserPanel.GetSelectedFilePath());
				auto spriteEditor = InGui::GetWindow(PanelID::SpriteEditor);
				auto [mx, my] = Input::GetMousePosition();
				auto [width, height] = Input::GetWindowSize();

				glm::vec2 mousePos = MouseToWorld({ mx,my }, { width,height });
				if (Collide(spriteEditor->Position, spriteEditor->Size, mousePos))
					m_SpriteEditorPanel.SetContext(texture);
			}
		}
		return false;
	}
	bool EditorLayer::onKeyPress(KeyPressedEvent& event)
	{	
		if (m_SelectedEntity)
		{
			if (event.IsKeyPressed(KeyCode::XYZ_KEY_DELETE))
			{
				m_SceneHierarchyPanel.RemoveEntity(m_SelectedEntity);
				m_SceneHierarchyPanel.InvalidateEntity();
				m_EntityInspectorLayout.SetContext(Entity());
				m_Scene->DestroyEntity(m_SelectedEntity);
				m_Scene->SetSelectedEntity(Entity());
				m_SelectedEntity = Entity();
				InGui::GetWindow(PanelID::SceneHierarchy)->Flags |= InGuiWindowFlag::Modified;
			}
			else if (event.IsKeyPressed(KeyCode::XYZ_KEY_S))
			{
				m_StartMousePos = InGui::GetWorldPosition(*InGui::GetWindow(PanelID::Scene), m_EditorCamera.GetPosition(), m_EditorCamera.GetAspectRatio(), m_EditorCamera.GetZoomLevel());
				m_ScalingEntity = true;
				m_ModifiedTransform = m_SelectedEntity.GetComponent<TransformComponent>();

				m_ModifiedTranslation = m_ModifiedTransform->Translation;
				m_ModifiedScale = m_ModifiedTransform->Scale;
				m_ModifiedRotation = m_ModifiedTransform->Rotation;

				// Remove entity;
				m_SceneHierarchyPanel.RemoveEntity(m_SelectedEntity);
			}
			else if (event.IsKeyPressed(KeyCode::XYZ_KEY_G))
			{
				m_StartMousePos = InGui::GetWorldPosition(*InGui::GetWindow(PanelID::Scene), m_EditorCamera.GetPosition(), m_EditorCamera.GetAspectRatio(), m_EditorCamera.GetZoomLevel());
				m_MovingEntity = true;
				m_ModifiedTransform = m_SelectedEntity.GetComponent<TransformComponent>();

				m_ModifiedTranslation = m_ModifiedTransform->Translation;
				m_ModifiedScale = m_ModifiedTransform->Scale;
				m_ModifiedRotation = m_ModifiedTransform->Rotation;

				// Remove entity;
				m_SceneHierarchyPanel.RemoveEntity(m_SelectedEntity);
			}
			else if (event.IsKeyPressed(KeyCode::XYZ_KEY_R))
			{
				m_StartMousePos = InGui::GetWorldPosition(*InGui::GetWindow(PanelID::Scene), m_EditorCamera.GetPosition(), m_EditorCamera.GetAspectRatio(), m_EditorCamera.GetZoomLevel());
				m_RotatingEntity = true;
				m_ModifiedTransform = m_SelectedEntity.GetComponent<TransformComponent>();
				m_ModifiedTranslation = m_ModifiedTransform->Translation;
				m_ModifiedScale = m_ModifiedTransform->Scale;
				m_ModifiedRotation = m_ModifiedTransform->Rotation;
			}
			
		}
		return false;
	}
	bool EditorLayer::onKeyRelease(KeyReleasedEvent& event)
	{
		if (event.IsKeyReleased(KeyCode::XYZ_KEY_S))
		{
			m_ScalingEntity = false;	
			if (m_SelectedEntity)
				m_SceneHierarchyPanel.InsertEntity(m_SelectedEntity);
		}
		else if (event.IsKeyReleased(KeyCode::XYZ_KEY_G))
		{
			m_MovingEntity = false;
			if (m_SelectedEntity)
				m_SceneHierarchyPanel.InsertEntity(m_SelectedEntity);
		}
		else if (event.IsKeyReleased(KeyCode::XYZ_KEY_R))
		{
			m_RotatingEntity = false;
		}
		return false;
	}
	void EditorLayer::onResizeSceneWindow(const glm::vec2& size)
	{
		m_EditorCamera.OnResize(size);
	}
	
}