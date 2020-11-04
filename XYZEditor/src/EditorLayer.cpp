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
		m_SpriteEditorPanel(PanelID::SpriteEditor),
		m_ScenePanel(PanelID::Scene),
		m_SceneHierarchyPanel(PanelID::SceneHierarchy),
		m_InspectorPanel(PanelID::InspectorPanel),
		m_AnimatorPanel(PanelID::AnimatorPanel),
		m_ProjectBrowserPanel(PanelID::ProjectBrowser)
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
		m_ScenePanel.SetContext(m_Scene);
		m_SceneHierarchyPanel.SetContext(m_Scene);

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

		


		InGui::SetUIOffset(10.0f);
		InGui::Begin(PanelID::Test, "Test", { 0,0 }, { 200,200 });
		InGui::End();
		InGui::GetWindow(PanelID::Test)->Flags |= (InGuiWindowFlag::MenuEnabled | InGuiWindowFlag::Visible);


		float divisor = 8.0f;
		auto& renderConfig = InGui::GetRenderConfiguration();
		renderConfig.SubTexture[PLAY] = Ref<SubTexture2D>::Create(renderConfig.Texture, glm::vec2(2, 2), glm::vec2(renderConfig.Texture->GetWidth() / divisor, renderConfig.Texture->GetHeight() / divisor));
		renderConfig.SubTexture[PAUSE] = Ref<SubTexture2D>::Create(renderConfig.Texture, glm::vec2(2, 1), glm::vec2(renderConfig.Texture->GetWidth() / divisor, renderConfig.Texture->GetHeight() / divisor));

		auto backgroundTexture = m_AssetManager.GetAsset<Texture2D>("Assets/Textures/Backgroundfield.png");
		m_SceneHierarchyPanel.SetContext(m_Scene);
		m_SpriteEditorPanel.SetContext(backgroundTexture);
		m_AnimatorPanel.SetContext(m_AnimationController);	

		m_ScenePanel.RegisterCallback(Hook(&EditorLayer::OnEvent, this));
		m_SceneHierarchyPanel.RegisterCallback(Hook(&EditorLayer::OnEvent, this));
		m_AnimatorPanel.RegisterCallback(Hook(&EditorLayer::OnEvent, this));
		m_SpriteEditorPanel.RegisterCallback(Hook(&EditorLayer::OnEvent, this));




		///////////////////////////////////////////////////////////

		uint32_t count = 5000;
		auto computeMat = Ref<Material>::Create(Shader::Create("Assets/Shaders/Particle/ParticleComputeShader.glsl"));
		auto renderMat = Ref<Material>::Create(Shader::Create("Assets/Shaders/Particle/ParticleShader.glsl"));
		
		renderMat->Set("u_Texture", Texture2D::Create(XYZ::TextureWrap::Clamp, TextureParam::Nearest, TextureParam::Nearest, "Assets/Textures/flame.png"), 0);
		renderMat->Set("u_Texture", Texture2D::Create(XYZ::TextureWrap::Clamp, TextureParam::Nearest, TextureParam::Nearest, "Assets/Textures/flame_emission.png"), 1);
	
		computeMat->SetRoutine("blueColor");	
	
		m_Particle = m_TestEntity.EmplaceComponent<ParticleComponent>(ParticleComponent());
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
			m_Vertices[i].Position = glm::vec4(0.008f * i, 0.0f, 0.0f, 1.0f);
			m_Vertices[i].Color = glm::vec4(0.3, 0.5, 1.0, 1);
			m_Vertices[i].Rotation = 0.0f;
			m_Vertices[i].TexCoordOffset = glm::vec2(0);

			m_Data[i].DefaultPosition.x = m_Vertices[i].Position.x;
			m_Data[i].DefaultPosition.y = m_Vertices[i].Position.y;
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
		
		m_ScenePanel.OnUpdate(ts);
		m_SceneHierarchyPanel.OnUpdate(ts);
		m_AnimatorPanel.OnUpdate(ts);
		m_SpriteEditorPanel.OnUpdate(ts);
		m_ProjectBrowserPanel.OnUpdate(ts);		
	}
	void EditorLayer::OnEvent(Event& event)
	{		
		m_AnimatorPanel.OnEvent(event);
		m_SpriteEditorPanel.OnEvent(event);
		m_ProjectBrowserPanel.OnEvent(event);
		m_ScenePanel.OnEvent(event);
		m_SceneHierarchyPanel.OnEvent(event);
		m_InspectorPanel.OnEvent(event);

		EventDispatcher dispatcher(event);
		dispatcher.Dispatch<MouseButtonPressEvent>(Hook(&EditorLayer::onMouseButtonPress, this));
		dispatcher.Dispatch<MouseButtonReleaseEvent>(Hook(&EditorLayer::onMouseButtonRelease, this));	
		dispatcher.Dispatch<EntitySelectedEvent>(Hook(&EditorLayer::onEntitySelected, this));
		dispatcher.Dispatch<DeselectedEvent>(Hook(&EditorLayer::onDeselected, this));
		dispatcher.Dispatch<AnimatorSelectedEvent>(Hook(&EditorLayer::onAnimatorSelected, this));
		dispatcher.Dispatch<SpriteSelectedEvent>(Hook(&EditorLayer::onSpriteSelected, this));
	}

	void EditorLayer::OnInGuiRender(Timestep ts)
	{
		m_ScenePanel.OnInGuiRender();
		m_SceneHierarchyPanel.OnInGuiRender();
		m_AnimatorPanel.OnInGuiRender();
		m_SpriteEditorPanel.OnInGuiRender();
		m_InspectorPanel.OnInGuiRender();
		m_ProjectBrowserPanel.OnInGuiRender();


		InGui::BeginPanel(InGuiPanelType::Left, 300.0f, m_LeftPanel);
		//if (m_Dragging && m_ProjectBrowserPanel.GetSelectedFileIndex() != -1)
		//{
		//	auto& renderConfig = InGui::GetRenderConfiguration();
		//	auto [mx, my] = Input::GetMousePosition();
		//	auto [width, height] = Input::GetWindowSize();
		//	glm::vec2 size = { 25.0f,25.0f };
		//	glm::vec2 position = MouseToWorld({ mx,my }, { width,height }) - (size / 2.0f);
		//			
		//	//if (m_ProhibitedCursor && !m_EntityInspectorLayout.ValidExtension(m_ProjectBrowserPanel.GetSelectedFilePath()))
		//	//{
		//	//	Application::Get().GetWindow().SetCustomCursor(m_ProhibitedCursor);
		//	//}
		//	//else
		//	//{
		//	//	Application::Get().GetWindow().SetStandardCursor(WindowCursors::XYZ_ARROW_CURSOR);
		//	//}
		//}
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
				InGui::GetWindow(PanelID::Test)->Flags |= InGuiWindowFlag::Closed;
				//PerModuleInterface::g_pRuntimeObjectSystem->CompileAll(true);
			}
		}
		InGui::End();
	}


	bool EditorLayer::onMouseButtonPress(MouseButtonPressEvent& event)
	{
	
		return false;
	}
	bool EditorLayer::onMouseButtonRelease(MouseButtonReleaseEvent& event)
	{
		if (event.IsButtonReleased(MouseCode::XYZ_MOUSE_BUTTON_LEFT))
		{
			m_Dragging = false;
			Application::Get().GetWindow().SetStandardCursor(WindowCursors::XYZ_ARROW_CURSOR);
			
			//if (HasExtension(m_ProjectBrowserPanel.GetSelectedFilePath(), "png"))
			//{
			//	auto texture = m_AssetManager.GetAsset<Texture2D>(m_ProjectBrowserPanel.GetSelectedFilePath());
			//	auto spriteEditor = InGui::GetWindow(PanelID::SpriteEditor);
			//	auto [mx, my] = Input::GetMousePosition();
			//	auto [width, height] = Input::GetWindowSize();
			//
			//	glm::vec2 mousePos = MouseToWorld({ mx,my }, { width,height });
			//	if (Collide(spriteEditor->Position, spriteEditor->Size, mousePos))
			//		m_SpriteEditorPanel.SetContext(texture);
			//}
		}
		return false;
	}

	bool EditorLayer::onEntitySelected(EntitySelectedEvent& event)
	{
		m_InspectableEntity.SetContext(event.GetEntity());
		m_InspectorPanel.SetInspectable(&m_InspectableEntity);
		return true;
	}
	bool EditorLayer::onDeselected(DeselectedEvent& event)
	{
		m_InspectorPanel.SetInspectable(nullptr);
		return true;
	}
	bool EditorLayer::onAnimatorSelected(AnimatorSelectedEvent& event)
	{
		m_InspectableAnimator.SetContext(event.GetController());
		m_InspectorPanel.SetInspectable(&m_InspectableAnimator);
		return true;
	}

	bool EditorLayer::onSpriteSelected(SpriteSelectedEvent& event)
	{
		m_InspectableSprite.SetContext(event.GetSprite());
		m_InspectorPanel.SetInspectable(&m_InspectableSprite);
		return false;
	}
	
}