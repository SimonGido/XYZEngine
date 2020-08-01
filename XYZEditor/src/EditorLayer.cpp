#include "EditorLayer.h"

#include <glm/gtc/matrix_transform.hpp>

namespace XYZ {
	EditorLayer::~EditorLayer()
	{
	}


	void EditorLayer::OnAttach()
	{
		Renderer::Init();
		SortLayer::CreateLayer("Default");

		m_FBO = FrameBuffer::Create({ 800,800 });
		m_FBO->CreateColorAttachment(FrameBufferFormat::RGBA16F);
		m_FBO->CreateDepthAttachment();
		m_FBO->Resize();

		m_Scene = SceneManager::Get().CreateScene("Test");

		m_Material = Material::Create(XYZ::Shader::Create("TextureShader", "Assets/Shaders/DefaultShader.glsl"));

		m_GuiTexture = XYZ::Texture2D::Create(XYZ::TextureWrap::Clamp, "Assets/Textures/Gui/TexturePack.png");
		m_Material->Set("u_Texture", m_GuiTexture, 0);
		m_Material->Set("u_Texture", XYZ::Texture2D::Create(XYZ::TextureWrap::Clamp, "Assets/Textures/wall.png"), 1);
		m_Material->Set("u_Texture", XYZ::Texture2D::Create(XYZ::TextureWrap::Repeat, "Assets/Textures/background.png"), 2);


		m_CharacterTexture = Texture2D::Create(XYZ::TextureWrap::Clamp, "Assets/Textures/player_sprite.png");
		m_Material->Set("u_Texture", m_CharacterTexture, 0);
		m_Material->SetFlags(XYZ::RenderFlags::TransparentFlag);


		m_TextMaterial = Material::Create(XYZ::Shader::Create("TextShader", "Assets/Shaders/TextShader.glsl"));
		m_TextMaterial->Set("u_Texture", XYZ::Texture2D::Create(XYZ::TextureWrap::Clamp, "Assets/Font/Arial.png"), 0);
		m_TextMaterial->SetFlags(XYZ::RenderFlags::TransparentFlag);

		m_CharacterSubTexture = Ref<SubTexture2D>::Create(m_CharacterTexture, glm::vec2(0, 0), glm::vec2(m_CharacterTexture->GetWidth() / 8, m_CharacterTexture->GetHeight() / 3));
		m_Font = Ref<Font>::Create("Assets/Font/Arial.fnt");


		m_TestEntity = m_Scene->CreateEntity("Test Entity");
		m_TestEntity.AddComponent(RenderComponent2D{
			m_Material,
			MeshFactory::CreateSprite({1,1,1,1},m_CharacterSubTexture->GetTexCoords(),0),
			SortLayer::GetOrderValue("Default")
		});

		m_Transform = m_TestEntity.GetComponent<Transform2D>();


		for (int i = 1; i < 5; ++i)
		{
			Entity entity = m_Scene->CreateEntity("Test Child");
			entity.AddComponent(RenderComponent2D{
				m_Material,
				MeshFactory::CreateSprite({1,1,1,1},m_CharacterSubTexture->GetTexCoords(),0),	
				SortLayer::GetOrderValue("Default")
			});

			auto transform = entity.GetComponent<Transform2D>();
			transform->Translate({ i,0,0 });
			m_Scene->SetParent(m_TestEntity, entity);
		}		
	}

	void EditorLayer::OnDetach()
	{
	}
	void EditorLayer::OnUpdate(float ts)
	{
		RenderCommand::Clear();
		RenderCommand::SetClearColor(glm::vec4(0.2, 0.2, 0.5, 1));

	
		
		
		glm::vec2 winSize = { Input::GetWindowSize().first, Input::GetWindowSize().second };
		
		m_FBO->Bind();
		RenderCommand::Clear();
		RenderCommand::SetClearColor(glm::vec4(0.2, 0.2, 0.2, 1));
		m_Scene->OnRenderEditor(ts, { m_EditorCamera.GetViewProjectionMatrix(),winSize });
		m_FBO->Unbind();


		if (m_ActiveWindow)
		{
			m_EditorCamera.OnUpdate(ts);
			if (Input::IsKeyPressed(KeyCode::XYZ_KEY_UP))
			{
				m_Transform->Translate(glm::vec3(0, 0.005, 0));
			}
			else if (XYZ::Input::IsKeyPressed(KeyCode::XYZ_KEY_DOWN))
			{
				m_Transform->Translate(glm::vec3(0, -0.005, 0));
			}
			if (XYZ::Input::IsKeyPressed(KeyCode::XYZ_KEY_LEFT))
			{
				m_Transform->Translate(glm::vec3(-0.005, 0, 0));
			}
			else if (XYZ::Input::IsKeyPressed(KeyCode::XYZ_KEY_RIGHT))
			{
				m_Transform->Translate(glm::vec3(0.005, 0, 0));
			}

			if (XYZ::Input::IsKeyPressed(KeyCode::XYZ_KEY_1))
			{
				m_Transform->Rotate(0.1f);
			}
			else if (XYZ::Input::IsKeyPressed(KeyCode::XYZ_KEY_2))
			{
				m_Transform->Rotate(-0.1f);
			}

			if (XYZ::Input::IsKeyPressed(KeyCode::XYZ_KEY_3))
			{
				m_Transform->Scale({ 0.1,0.1 });
			}
			else if (XYZ::Input::IsKeyPressed(KeyCode::XYZ_KEY_4))
			{
				m_Transform->Scale({ -0.1,-0.1 });
			}
		}
	}
	void EditorLayer::OnEvent(Event& event)
	{
		m_EditorCamera.OnEvent(event);
	}

	void EditorLayer::OnInGuiRender()
	{
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

		if (InGui::Begin("Test Panel", { 0,0 }, { 500,500 }))
		{		
			
			InGui::ColorPicker4("color picker", { 255,255 },m_Pallete, m_Color);
			if (InGui::Button("Button", { 100,25 }))
			{
				std::cout << "Opica" << std::endl;
			}
			if (InGui::Button("Button", { 100,25 }))
			{
				std::cout << "Opica" << std::endl;
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
	
	
}