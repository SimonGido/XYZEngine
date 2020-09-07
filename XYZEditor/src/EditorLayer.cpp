#include "EditorLayer.h"


#include <glm/glm.hpp>
#include <glm/gtx/transform.hpp>



namespace XYZ {

	//static glm::vec2 GetWorldPositionFromInGui(const InGui::InGuiWindow &window, const EditorCamera& camera)
	//{
	//	auto [x, y] = Input::GetMousePosition();
	//	auto [width, height] = Input::GetWindowSize();
	//
	//	x -= (((float)width / 2) + window.Position.x);
	//
	//	float boundWidth = (camera.GetZoomLevel() * camera.GetAspectRatio()) * 2;
	//	float boundHeight = (camera.GetZoomLevel() * camera.GetAspectRatio()) * 2;
	//
	//	x = (x / window.Size.x) * boundWidth - boundWidth * 0.5f;
	//	y = boundHeight * 0.5f - (y / (window.Size.y + InGui::InGuiWindow::PanelSize)) * boundHeight;
	//
	//	return { x + camera.GetPosition().x ,y + camera.GetPosition().y };
	//}

	EditorLayer::~EditorLayer()
	{
	}

	void EditorLayer::OnAttach()
	{
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
		

		m_CharacterTexture = Texture2D::Create(XYZ::TextureWrap::Clamp, "Assets/Textures/player_sprite.png");
		m_CharacterSubTexture = Ref<SubTexture2D>::Create(m_CharacterTexture, glm::vec2(0, 0), glm::vec2(m_CharacterTexture->GetWidth() / 8, m_CharacterTexture->GetHeight() / 3));
		m_CharacterSubTexture2 = Ref<SubTexture2D>::Create(m_CharacterTexture, glm::vec2(1, 2), glm::vec2(m_CharacterTexture->GetWidth() / 8, m_CharacterTexture->GetHeight() / 3));
		m_CharacterSubTexture3 = Ref<SubTexture2D>::Create(m_CharacterTexture, glm::vec2(2, 2), glm::vec2(m_CharacterTexture->GetWidth() / 8, m_CharacterTexture->GetHeight() / 3));

		
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

		InGui::SetWindowFlags("scene", 0);
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
		RenderCommand::Clear();
		RenderCommand::SetClearColor(glm::vec4(0.2, 0.2, 0.2, 1));
		m_Scene->OnUpdate(ts);
		m_Scene->OnRenderEditor({ m_EditorCamera.GetViewProjectionMatrix(),winSize });
		
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
		dispatcher.Dispatch<MouseButtonPressEvent>(Hook(&EditorLayer::onMouseButtonPress, this));
		dispatcher.Dispatch<KeyPressedEvent>(Hook(&EditorLayer::onKeyPress, this));
		dispatcher.Dispatch<KeyReleasedEvent>(Hook(&EditorLayer::onKeyRelease, this));
		m_EditorCamera.OnEvent(event);
	}

	void EditorLayer::OnInGuiRender()
	{
		if (InGui::Begin("Scene hierarchy", { 0,0 }, { 200,200 }))
		{
			bool test = false;
			if (InGui::Checkbox("test", { 50,50 }, test))
			{
				std::cout << "Clicked" << std::endl;
			}
			if (InGui::Button("test", { 50,30 }))
			{
				std::cout << "Clicked" << std::endl;
			}
			if (InGui::Button("test", { 50,50 }))
			{
				std::cout << "Clicked" << std::endl;
			}
			if (InGui::Button("test", { 50,50 }))
			{
				std::cout << "Clicked" << std::endl;
			}
			if (InGui::Button("test", { 50,50 }))
			{
				std::cout << "Clicked" << std::endl;
			}
			if (InGui::Slider("Slider", { 150,15 }, m_H))
			{
				std::cout << m_H << std::endl;
			}
		}
		InGui::End();

		if (InGui::Begin("Entity", { 0,0 }, { 200,200 }))
		{

		}
		InGui::End();

		if (InGui::RenderWindow("Scene", m_FBO->GetColorAttachment(0).RendererID, { 0,0 }, { 200,200 }, 25.0f))
		{
			m_ActiveWindow = true;
		}
		else
		{
			m_ActiveWindow = false;
		}
		InGui::End();

		if (InGui::Begin("Test panel", { 0,0 }, { 200,200 }))
		{

		}
		InGui::End();

		if (InGui::Begin("Test", { 0,0 }, { 200,200 }))
		{

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
	bool EditorLayer::onMouseButtonPress(MouseButtonPressEvent& event)
	{
		if (event.IsButtonPressed(MouseCode::XYZ_MOUSE_BUTTON_LEFT))
		{
				
		}
		return false;
	}
	bool EditorLayer::onKeyPress(KeyPressedEvent& event)
	{
		if (event.IsKeyPressed(KeyCode::XYZ_KEY_S))
		{
			
		}
		else if (event.IsKeyPressed(KeyCode::XYZ_KEY_G))
		{
			
		}
		else if (event.IsKeyPressed(KeyCode::XYZ_KEY_R))
		{
			
		}
		return false;
	}
	bool EditorLayer::onKeyRelease(KeyReleasedEvent& event)
	{
		if (event.IsKeyReleased(KeyCode::XYZ_KEY_S))
		{
			m_ScalingEntity = false;		
			m_SceneHierarchyPanel.InsertEntity(m_SelectedEntity);
		}
		else if (event.IsKeyReleased(KeyCode::XYZ_KEY_G))
		{
			m_MovingEntity = false;
			m_SceneHierarchyPanel.InsertEntity(m_SelectedEntity);
		}
		else if (event.IsKeyReleased(KeyCode::XYZ_KEY_R))
		{
			m_RotatingEntity = false;
		}
		return false;
	}
}