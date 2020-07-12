#include "EditorLayer.h"

#include <glm/gtc/matrix_transform.hpp>

namespace XYZ {
	EditorLayer::~EditorLayer()
	{
	}


	void EditorLayer::OnAttach()
	{
		Renderer::Init();
		m_Scene = SceneManager::Get().CreateScene("Test");


		
		m_SortSystem = CreateRef<RenderSortSystem>();
		
		m_Material = Material::Create(XYZ::Shader::Create("TextureShader", "Assets/Shaders/DefaultShader.glsl"));
	
		m_GuiTexture = XYZ::Texture2D::Create(XYZ::TextureWrap::Clamp, "Assets/Textures/Gui/TexturePack.png");
		m_Material->Set("u_Texture", m_GuiTexture, 0);
		m_Material->Set("u_Texture", XYZ::Texture2D::Create(XYZ::TextureWrap::Clamp, "Assets/Textures/wall.png"), 1);
		m_Material->Set("u_Texture", XYZ::Texture2D::Create(XYZ::TextureWrap::Repeat, "Assets/Textures/background.png"), 2);
		
		
		Ref<Texture2D> texture = Texture2D::Create(XYZ::TextureWrap::Clamp, "Assets/Textures/player_sprite.png");
		m_Material->Set("u_Texture", texture, 3);
		m_Material->SetFlags(XYZ::RenderFlags::TransparentFlag);
		

		m_TextMaterial = Material::Create(XYZ::Shader::Create("TextShader", "Assets/Shaders/TextShader.glsl"));
		m_TextMaterial->Set("u_Texture", XYZ::Texture2D::Create(XYZ::TextureWrap::Clamp, "Assets/Font/Arial.png"), 0);
		m_TextMaterial->SetFlags(XYZ::RenderFlags::TransparentFlag);

		Ref<SubTexture2D> subTexture = CreateRef<SubTexture2D>(texture, glm::vec2(0, 0), glm::vec2(texture->GetWidth() / 8, texture->GetHeight() / 3));


		m_Renderable = m_TestObject.AddComponent(SpriteRenderComponent(
			glm::vec2(0, 0),
			glm::vec2(1, 1),
			glm::vec4(1),
			3,
			subTexture,
			m_Material,
			SortingLayer::Get().GetOrderValue("Default"),
			true
		));
		
		m_Transform = m_TestObject.AddComponent(Transform2D{
				glm::vec3(0,0,0)
			});

		
		m_ChildRenderable = m_TestChild.AddComponent(SpriteRenderComponent(
			glm::vec2(0, 0),
			glm::vec2(1, 1),
			glm::vec4(1),
			3,
			subTexture,
			m_Material,
			SortingLayer::Get().GetOrderValue("Default"),
			true
		));

		m_ChildTransform = m_TestChild.AddComponent(Transform2D{
			glm::vec3(0,1,0)
			});

	
		m_ChildRenderable2 = m_TestChild2.AddComponent(SpriteRenderComponent(
			glm::vec2(0, 0),
			glm::vec2(1, 1),
			glm::vec4(1),
			3,
			subTexture,
			m_Material,
			SortingLayer::Get().GetOrderValue("Default"),
			true
		));
		
		m_ChildTransform2 = m_TestChild2.AddComponent(Transform2D{
			glm::vec3(1,1,0)
		});

		

		m_WorldTransform = new Transform2D(glm::vec3(0, 0, 0));
		m_SceneGraph.InsertNode(Node<SceneObject>({ nullptr,m_WorldTransform }));
		m_SceneGraph.InsertNode(Node<SceneObject>({ m_Renderable,m_Transform }));
		m_SceneGraph.InsertNode(Node<SceneObject>({ m_ChildRenderable,m_ChildTransform }));
		m_SceneGraph.InsertNode(Node<SceneObject>({ m_ChildRenderable2,m_ChildTransform2 }));
		
		m_SceneGraph.SetRoot(0);
		m_SceneGraph.SetParent(0, 1, SceneSetup());
		m_SceneGraph.SetParent(0, 2, SceneSetup());
		m_SceneGraph.SetParent(0, 3, SceneSetup());
		
		m_Font = CreateRef<Font>("Assets/Font/Arial.fnt");


		m_TextRenderable = m_Text.AddComponent(Text(
			"A sample string of text!",
			glm::vec2(0, 0),
			glm::vec4(1),
			0,
			m_Font,
			m_TextMaterial,
			SortingLayer::Get().GetOrderValue("Default"),
			true
		));


		m_TextTransform = m_Text.AddComponent(Transform2D(
			glm::vec3(1,1,0) 
		));

		m_TextTransform->SetScale(glm::vec2(1.0f, 1.0f));
		m_SceneGraph.InsertNode(Node<SceneObject>({ m_TextRenderable,m_TextTransform }));
		m_SceneGraph.SetParent(1, 4, SceneSetup());
	



		m_ButtonTransform = m_Button.AddComponent(Transform2D(
			glm::vec3(0, 0, 0),
			glm::vec2(1, 1)
		));

		Ref<SubTexture2D> buttonSubTexture = CreateRef<SubTexture2D>(m_GuiTexture, glm::vec2(0, 0), glm::vec2(m_GuiTexture->GetWidth() / 4, m_GuiTexture->GetHeight() / 4));
		m_ButtonRenderable = m_Button.AddComponent(Image(
			glm::vec2(0, 0),
			glm::vec2(1, 1),
			glm::vec4(1),
			0,
			buttonSubTexture,
			m_Material,
			SortingLayer::Get().GetOrderValue("Default"),
			true
		));


		m_SceneGraph.InsertNode(Node<SceneObject>({ m_ButtonRenderable,m_ButtonTransform }));
		m_SceneGraph.SetParent(4, 5, SceneSetup());
		
		m_ButtonComponent = m_Button.AddComponent(Button());		

		m_ButtonComponent->RegisterCallback<ClickEvent>([this](ClickEvent e) {
			((Image*)m_ButtonRenderable)->SetColor({ 1,0,0,1 });
		});

		m_ButtonComponent->RegisterCallback<ReleaseEvent>([this](ReleaseEvent e) {
			((Image*)m_ButtonRenderable)->SetColor({ 1,1,1,1 });
		});

		m_CheckboxTransform = m_Checkbox.AddComponent(Transform2D(
			glm::vec3(2, 0, 0),
			glm::vec2(1, 1)
		));

		m_CheckboxSubTexture = CreateRef<SubTexture2D>(m_GuiTexture, glm::vec2(0, 1), glm::vec2(m_GuiTexture->GetWidth() / 4, m_GuiTexture->GetHeight() / 4));
		m_CheckboxRenderable = m_Checkbox.AddComponent(Image(
			glm::vec2(0, 0),
			glm::vec2(1, 1),
			glm::vec4(1),
			0,
			m_CheckboxSubTexture,
			m_Material,
			SortingLayer::Get().GetOrderValue("Default"),
			true
		));


		m_SceneGraph.InsertNode(Node<SceneObject>({ m_CheckboxRenderable,m_CheckboxTransform }));
		m_SceneGraph.SetParent(4, 6, SceneSetup());

		m_CheckboxComponent = m_Checkbox.AddComponent(Checkbox());


		m_CheckboxComponent->RegisterCallback<ClickEvent>([this](ClickEvent e) {
			m_CheckboxSubTexture->SetCoords({ 1,1 }, { m_GuiTexture->GetWidth() / 4, m_GuiTexture->GetHeight() / 4 });
			((Image*)m_CheckboxRenderable)->SetSubTexture(m_CheckboxSubTexture);
		});

		m_CheckboxComponent->RegisterCallback<HooverEvent>([this](HooverEvent e) {
			((Image*)m_CheckboxRenderable)->SetColor({ 1, 0, 0, 1 });
		});

		m_CheckboxComponent->RegisterCallback<UnHooverEvent>([this](UnHooverEvent e) {
			((Image*)m_CheckboxRenderable)->SetColor({ 1, 1, 1, 1 });
		});

		m_CheckboxComponent->RegisterCallback<ReleaseEvent>([this](ReleaseEvent e) {
			m_CheckboxSubTexture->SetCoords({ 0,1 }, { m_GuiTexture->GetWidth() / 4, m_GuiTexture->GetHeight() / 4 });
			((Image*)m_CheckboxRenderable)->SetSubTexture(m_CheckboxSubTexture);
		});

		for (int i = 0; i < 100; ++i)
		{
			Entity* entity = new Entity();
			auto render = entity->AddComponent(SpriteRenderComponent(
				glm::vec2(0, 0),
				glm::vec2(1, 1),
				glm::vec4(1),
				3,
				subTexture,
				m_Material,
				SortingLayer::Get().GetOrderValue("Default"),
				true
			));

			auto trans = entity->AddComponent(Transform2D{
				glm::vec3(i,1,0)
				});

			m_SceneGraph.InsertNode(Node<SceneObject>({ render,trans }));
			m_SceneGraph.SetParent(1, i+7, SceneSetup());
		}

		// Should be called anyways root of scene graph is always updated
		m_SceneGraph.GetFlatData()[0].GetData().Transform->CalculateWorldTransformation();
		m_SceneGraph.GetFlatData()[0].GetData().Transform->GetTransformation();
	}
	void EditorLayer::OnDetach()
	{
	}
	void EditorLayer::OnUpdate(float ts)
	{
		RenderCommand::Clear();
		RenderCommand::SetClearColor(glm::vec4(0.2, 0.2, 0.5, 1));

		m_EditorCamera.OnUpdate(ts);
	
		Renderer2D::BeginScene(m_EditorCamera.GetViewProjectionMatrix());
		while (m_SceneGraph.Next())
		{
			auto it = m_SceneGraph.Iterator();
			it->GetData().Transform->CalculateWorldTransformation();
			if (it->GetData().Renderable)
				m_SortSystem->PushRenderData(it->GetData().Renderable, it->GetData().Transform);
		}
		
		m_SortSystem->SubmitToRenderer();
		Renderer2D::EndScene();


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
			m_Transform->Scale({ 0.01,0.01 });
		}
		else if (XYZ::Input::IsKeyPressed(KeyCode::XYZ_KEY_4))
		{
			m_Transform->Scale({ -0.01,-0.01 });
		}
	}
	void EditorLayer::OnEvent(Event& event)
	{
		m_EditorCamera.OnEvent(event);
	}
	
}