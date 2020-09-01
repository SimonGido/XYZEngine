#include "EditorLayer.h"


#include <glm/glm.hpp>
#include <glm/gtx/transform.hpp>


namespace XYZ {

	EditorLayer::~EditorLayer()
	{
	}


	void EditorLayer::OnAttach()
	{

		Renderer::Init();
		NativeScriptEngine::Init();

		NativeScriptEngine::SetOnReloadCallback([this] {
			auto storage = m_Scene->GetECS()->GetComponentStorage<NativeScriptComponent>();
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
			auto storage = m_Scene->GetECS()->GetComponentStorage<NativeScriptComponent>();
			for (int i = 0; i < storage->Size(); ++i)
			{
				if ((*storage)[i].ScriptableEntity)
				{
					m_StoredEntitiesWithScript.push_back((*storage)[i].ScriptableEntity->Entity);
				}
			}
		});
		

		auto& app = Application::Get();
		m_FBO = FrameBuffer::Create({ app.GetWindow().GetWidth(),app.GetWindow().GetHeight() });
		m_FBO->CreateColorAttachment(FrameBufferFormat::RGBA16F);
		m_FBO->CreateDepthAttachment();
		m_FBO->Resize();

		m_Scene = m_AssetManager.GetAsset<Scene>("Assets/Scenes/scene.xyz");
		SceneManager::Get().SetActive(m_Scene);

		m_Material = m_AssetManager.GetAsset<Material>("Assets/Materials/material.mat");
		m_Material->SetFlags(XYZ::RenderFlags::TransparentFlag);

		m_TestEntity = m_Scene->GetEntity(2);
		m_TestEntity2 = m_Scene->GetEntity(3);
		m_SpriteRenderer = m_TestEntity.GetComponent<SpriteRenderer>();
		m_Transform = m_TestEntity.GetComponent<TransformComponent>();
		
		

		//m_TextMaterial = Material::Create(XYZ::Shader::Create("TextShader", "Assets/Shaders/TextShader.glsl"));
		//m_TextMaterial->Set("u_Texture", XYZ::Texture2D::Create(XYZ::TextureWrap::Clamp, "Assets/Font/Arial.png"), 0);
		//m_TextMaterial->SetFlags(XYZ::RenderFlags::TransparentFlag);


		m_CharacterTexture = Texture2D::Create(XYZ::TextureWrap::Clamp, "Assets/Textures/player_sprite.png");
		m_CharacterSubTexture = Ref<SubTexture2D>::Create(m_CharacterTexture, glm::vec2(0, 0), glm::vec2(m_CharacterTexture->GetWidth() / 8, m_CharacterTexture->GetHeight() / 3));
		m_CharacterSubTexture2 = Ref<SubTexture2D>::Create(m_CharacterTexture, glm::vec2(1, 2), glm::vec2(m_CharacterTexture->GetWidth() / 8, m_CharacterTexture->GetHeight() / 3));
		m_CharacterSubTexture3 = Ref<SubTexture2D>::Create(m_CharacterTexture, glm::vec2(2, 2), glm::vec2(m_CharacterTexture->GetWidth() / 8, m_CharacterTexture->GetHeight() / 3));

		//m_Font = Ref<Font>::Create("Assets/Font/Arial.fnt");
		//
		//m_TestEntity = m_Scene->CreateEntity("Test Entity");
		//m_TestEntity.AddComponent(SpriteRenderer{
		//		m_Material,
		//		m_CharacterSubTexture,
		//		0,
		//		SortLayer::GetOrderValue("Default")
		//});
		//
		//m_Transform = m_TestEntity.GetComponent<Transform>();
		//
		//
		//
		//for (int i = 1; i < 50000; ++i)
		//{
		//	Entity entity = m_Scene->CreateEntity("Test Child");
		//	entity.AddComponent(SpriteRenderer{
		//		m_Material,
		//		m_CharacterSubTexture,	
		//		0,
		//		SortLayer::GetOrderValue("Default")
		//	});
		//
		//	auto transform = entity.GetComponent<Transform>();
		//	transform->Translate({ i,0,0 });
		//	m_Scene->SetParent(m_TestEntity, entity);
		//}	


		m_Animation = new Animation(3.0f);


		auto prop = m_Animation->AddProperty<glm::vec4>(m_SpriteRenderer->Color);
		auto posProperty = m_Animation->AddProperty<glm::vec3>(m_Position);
		auto rotProperty = m_Animation->AddProperty<glm::vec3>(m_Rotation);
		auto spriteProperty = m_Animation->AddProperty<Ref<SubTexture2D>>(m_SpriteRenderer->SubTexture);
		
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


		m_RunAnimation = new Animation(3.0f);
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

		m_Machine = new Machine<Animation*>("Idle", m_Animation);
		m_Machine->AddState("Run", m_RunAnimation);

		m_Machine->SetStateTransition("Idle", "Run");
		m_Machine->SetStateTransition("Run", "Idle");


		m_SceneHierarchyPanel.SetContext(m_Scene);
	}

	void EditorLayer::OnDetach()
	{
		delete m_Animation;
		delete m_RunAnimation;
		delete m_Machine;

		NativeScriptEngine::Shutdown();
	}
	void EditorLayer::OnUpdate(Timestep ts)
	{
		NativeScriptEngine::Update(ts);
			
	
		RenderCommand::Clear();
		RenderCommand::SetClearColor(glm::vec4(0.2, 0.2, 0.5, 1));

			
		glm::vec2 winSize = { Input::GetWindowSize().first, Input::GetWindowSize().second };
		
		m_FBO->Bind();
		RenderCommand::Clear();
		RenderCommand::SetClearColor(glm::vec4(0.2, 0.2, 0.2, 1));
		m_Scene->OnUpdate(ts);
		m_Scene->OnRenderEditor({ m_EditorCamera.GetViewProjectionMatrix(),winSize });
		m_FBO->Unbind();

		if (m_ActiveWindow)
		{
			m_EditorCamera.OnUpdate(ts);		
		}

		if (Input::IsKeyPressed(KeyCode::XYZ_KEY_LEFT))
		{
			m_Machine->TransitionTo("Run");
		}
		else if (Input::IsKeyPressed(KeyCode::XYZ_KEY_RIGHT))
		{
			m_Machine->TransitionTo("Idle");
		}

		//m_Animation->Update(ts);
		//m_Machine->GetCurrentState().Value->Update(ts);
		//*m_Transform = glm::translate(glm::mat4(1.0f), m_Position) *
		//	glm::rotate(m_Rotation.z, glm::vec3(0, 0, 1)) * glm::scale(glm::mat4(1.0f), { 1,1,1 });
	}
	void EditorLayer::OnEvent(Event& event)
	{
		EventDispatcher dispatcher(event);
		dispatcher.Dispatch<WindowResizeEvent>(Hook(&EditorLayer::onWindowResized, this));
		m_EditorCamera.OnEvent(event);
	}

	void EditorLayer::OnInGuiRender()
	{
		m_SceneHierarchyPanel.OnInGuiRender();
		m_EntityComponentPanel.SetContext(m_SceneHierarchyPanel.GetSelectedEntity());
		m_EntityComponentPanel.OnInGuiRender();

		if (InGui::RenderWindow("Scene", m_FBO->GetColorAttachment(0).RendererID, { 0,-300 }, { 800,800 }, 25.0f))
		{
			m_ActiveWindow = true;
			InGui::Selector();
		}
		else
		{
			m_ActiveWindow = false;
		}
		InGui::End();
		if (InGui::Begin("Test", { 0,0 }, { 500,500 }))
		{
			if (InGui::BeginPopup("Add Component", { 150,25 }, m_PopupOpen))
			{
				if (InGui::PopupItem("Add Transform", { 150,25 }))
				{
					std::cout << "Adding transform" << std::endl;
					m_PopupOpen = false;
				}
			}
			InGui::EndPopup();
		
			if (InGui::TextArea("Test",m_Text, { 150,25 }, m_Modified))
			{
				std::cout << atof(m_Text.c_str()) << std::endl;
			}
		}
		
		InGui::BeginMenu();
		if (InGui::MenuBar("File", m_MenuOpen))
		{
			if (InGui::MenuItem("Load Scene", { 150,25 }))
			{
				auto& app = Application::Get();
				std::string filepath = app.OpenFile("(*.xyz)\0*.xyz\0");
				if (!filepath.empty())
				{
					m_Scene = m_AssetManager.GetAsset<Scene>(filepath);
				}
				m_MenuOpen = false;
			}
			else if (InGui::MenuItem("Create Script", { 150,25 }))
			{
				auto& app = Application::Get();
				std::string filepath = app.CreateNewFile("(*.cpp)\0*.cpp\0");
				if (!filepath.empty())
				{
					PerModuleInterface::g_pRuntimeObjectSystem->AddToRuntimeFileList(filepath.c_str());
				}
				m_MenuOpen = false;
			}
			else if (InGui::MenuItem("Load Script", { 150,25 }))
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
		InGui::MenuBarEnd();
		InGui::MenuBar("Settings", m_MenuOpen);
		InGui::MenuBarEnd();
		
		InGui::MenuBar("Settingass", m_MenuOpen);
		InGui::MenuBarEnd();
		InGui::MenuBar("Settingasdas", m_MenuOpen);
		InGui::MenuBarEnd();
		InGui::EndMenu();
		
		
		
		
		InGui::End();
		if (InGui::Begin("Test Panel", { 0,0 }, { 500,500 }))
		{			
			InGui::ColorPicker4("color picker", { 255,255 },m_Pallete, m_Color);
			if (InGui::Button("Compile", { 100,25 }))
			{
				PerModuleInterface::g_pRuntimeObjectSystem->CompileAll(true);
			}
			if (InGui::Button("Add Script", { 150,25 }))
			{
				ScriptableEntity* scriptableEntity = (ScriptableEntity*)NativeScriptEngine::CreateScriptObject("Testik");
				NativeScriptComponent comp(scriptableEntity, "Testik");
				comp.ScriptableEntity->Entity = m_TestEntity;
				comp.ScriptableEntity->OnCreate();
				m_TestEntity.AddComponent<NativeScriptComponent>(comp);
			}
			if (InGui::Button("Button", { 100,25 }))
			{
				std::cout << "Opica" << std::endl;
			}
			if (InGui::Checkbox("Checkbox", { 50,50 }, m_CheckboxVal))
			{
				std::cout << "Wtf" << std::endl;
			}
			if (InGui::Slider("Slider", { 200,15 }, m_TestValue))
			{
				std::cout << m_TestValue << std::endl;
			}
			if (InGui::Slider("Slider", { 200,15 }, m_TestValue))
			{
				std::cout << m_TestValue << std::endl;
			}
			if (InGui::Slider("Slider", { 200,15 }, m_TestValue))
			{
				std::cout << m_TestValue << std::endl;
			}
			if (InGui::Checkbox("Checkbox", { 50,50 }, m_CheckboxVal))
			{
				std::cout << "Wtf" << std::endl;
			}
			if (InGui::Checkbox("Checkbox", { 50,50 }, m_CheckboxVal))
			{
				std::cout << "Wtf" << std::endl;
			}
			if (InGui::Checkbox("Checkbox", { 50,50 }, m_CheckboxVal))
			{
				std::cout << "Wtf" << std::endl;
			}
			InGui::Image("test image", m_FBO->GetColorAttachment(0).RendererID, { 100,100 });
		}
		InGui::End();
	}

	bool EditorLayer::onWindowResized(WindowResizeEvent& event)
	{
		auto specs = m_FBO->GetSpecification();
		specs.Width = event.GetWidth();
		specs.Height = event.GetHeight();
		m_FBO->SetSpecification(specs);
		m_FBO->Resize();

		return false;
	}
}