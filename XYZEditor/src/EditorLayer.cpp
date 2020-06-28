#include "EditorLayer.h"



namespace XYZ {
	EditorLayer::~EditorLayer()
	{
	}


	void EditorLayer::OnAttach()
	{
		Renderer::Init();
		m_SortSystem = CreateRef<RenderSortSystem>();

		m_CameraController = CreateRef<OrthoCameraController>(16.0f / 8.0f, false);
		m_Material = Material::Create(XYZ::Shader::Create("TextureShader", "Assets/Shaders/DefaultShader.glsl"));
	
		m_Material->Set("u_Texture", XYZ::Texture2D::Create(XYZ::TextureWrap::Clamp, "Assets/Textures/wall.png"), 0);
		m_Material->Set("u_Texture", XYZ::Texture2D::Create(XYZ::TextureWrap::Clamp, "Assets/Textures/wall.png"), 1);
		m_Material->Set("u_Texture", XYZ::Texture2D::Create(XYZ::TextureWrap::Repeat, "Assets/Textures/background.png"), 2);
		
		
		Ref<Texture2D> texture = Texture2D::Create(XYZ::TextureWrap::Clamp, "Assets/Textures/player_sprite.png");
		m_Material->Set("u_Texture", texture, 3);
		m_Material->Set("u_ViewProjection", m_CameraController->GetCamera().GetViewProjectionMatrix());
		m_Material->SetFlags(XYZ::RenderFlags::TransparentFlag);
		

		m_TextMaterial = Material::Create(XYZ::Shader::Create("TextShader", "Assets/Shaders/TextShader.glsl"));
		m_TextMaterial->Set("u_Texture", XYZ::Texture2D::Create(XYZ::TextureWrap::Clamp, "Assets/Font/Calibri.png"), 0);
		m_TextMaterial->SetFlags(XYZ::RenderFlags::TransparentFlag);

		m_TestObject = ECSManager::Get().CreateEntity();;
		ECSManager::Get().AddComponent<SpriteRenderComponent>(m_TestObject, SpriteRenderComponent(
			glm::vec2(0,0),
			glm::vec2(1,1),
			glm::vec4(1),
			3,
			m_Material,
			SortingLayer::Get().GetOrderValue("Default"),
			true
		));

		

		ECSManager::Get().AddComponent<XYZ::Transform2D>(m_TestObject, XYZ::Transform2D{
		glm::vec3(0,0,0)
			});


		m_TestChild = ECSManager::Get().CreateEntity();;
		ECSManager::Get().AddComponent<SpriteRenderComponent>(m_TestChild, SpriteRenderComponent(
			glm::vec2(0, 0),
			glm::vec2(1, 1),
			glm::vec4(1),
			3,
			m_Material,
			SortingLayer::Get().GetOrderValue("Default"),
			true
		));

		ECSManager::Get().AddComponent<XYZ::Transform2D>(m_TestChild, XYZ::Transform2D{
			glm::vec3(0,1,0)
			});


		m_TestChild2 = ECSManager::Get().CreateEntity();;
		ECSManager::Get().AddComponent<SpriteRenderComponent>(m_TestChild2, SpriteRenderComponent(
			glm::vec2(0, 0),
			glm::vec2(1, 1),
			glm::vec4(1),
			3,
			m_Material,
			SortingLayer::Get().GetOrderValue("Default"),
			true
		));

		ECSManager::Get().AddComponent<XYZ::Transform2D>(m_TestChild2, XYZ::Transform2D{
		glm::vec3(1,1,0)
			});

		m_WorldTransform = new Transform2D(glm::vec3(0, 0, 0));
		m_Transform = ECSManager::Get().GetComponent<Transform2D>(m_TestObject);
		m_Renderable = ECSManager::Get().GetComponent<SpriteRenderComponent>(m_TestObject);
		((SpriteRenderComponent*)m_Renderable)->SetSubTexture(CreateRef<SubTexture2D>(texture, glm::vec2(0, 0), glm::vec2(texture->GetWidth() / 8, texture->GetHeight() / 3)));
	

		m_ChildTransform = ECSManager::Get().GetComponent<Transform2D>(m_TestChild);
		m_ChildRenderable = ECSManager::Get().GetComponent<SpriteRenderComponent>(m_TestChild);
		((SpriteRenderComponent*)m_ChildRenderable)->SetSubTexture(CreateRef<SubTexture2D>(texture, glm::vec2(0, 0), glm::vec2(texture->GetWidth() / 8, texture->GetHeight() / 3)));
		


		m_ChildTransform2 = ECSManager::Get().GetComponent<Transform2D>(m_TestChild2);
		m_ChildRenderable2 = ECSManager::Get().GetComponent<SpriteRenderComponent>(m_TestChild2);
		((SpriteRenderComponent*)m_ChildRenderable2)->SetSubTexture(CreateRef<SubTexture2D>(texture, glm::vec2(0, 0), glm::vec2(texture->GetWidth() / 8, texture->GetHeight() / 3)));



		m_SceneGraph.InsertNode(Node<SceneObject>({ nullptr,m_WorldTransform }));
		m_SceneGraph.InsertNode(Node<SceneObject>({ m_Renderable,m_Transform }));
		m_SceneGraph.InsertNode(Node<SceneObject>({ m_ChildRenderable,m_ChildTransform }));
		m_SceneGraph.InsertNode(Node<SceneObject>({ m_ChildRenderable2,m_ChildTransform2 }));
		
		m_SceneGraph.SetRoot(0);
		m_SceneGraph.SetParent(0, 1, SceneSetup());
		m_SceneGraph.SetParent(1, 2, SceneSetup());
		m_SceneGraph.SetParent(1, 3, SceneSetup());
		
		m_Font = CreateRef<Font>("Assets/Font/Calibri.fnt");


		m_Text = ECSManager::Get().CreateEntity();
		ECSManager::Get().AddComponent<Text>(m_Text, Text(
			"A sample string of text!",
			glm::vec2(0,0),
			glm::vec4(1),
			0,
			m_Font,
			m_TextMaterial,
			SortingLayer::Get().GetOrderValue("Default"),
			true
		));
		ECSManager::Get().AddComponent <Transform2D>(m_Text, Transform2D(
			glm::vec3(1,1,0) 
		));
		m_TextTransform = ECSManager::Get().GetComponent<Transform2D>(m_Text);
		m_TextRenderable = ECSManager::Get().GetComponent<Text>(m_Text);
		m_TextTransform->SetScale(glm::vec2(1.0f / 60.0f, 1.0f / 60.0f));
		m_SceneGraph.InsertNode(Node<SceneObject>({ m_TextRenderable,m_TextTransform }));
		m_SceneGraph.SetParent(1, 4, SceneSetup());
	}
	void EditorLayer::OnDetach()
	{
	}
	void EditorLayer::OnUpdate(float ts)
	{
		RenderCommand::Clear();
		RenderCommand::SetClearColor(glm::vec4(0.2, 0.2, 0.5, 1));
		m_CameraController->OnUpdate(ts);
		m_Material->Set("u_ViewProjection", m_CameraController->GetCamera().GetViewProjectionMatrix());
		m_TextMaterial->Set("u_ViewProjection", m_CameraController->GetCamera().GetViewProjectionMatrix());
		m_SceneGraph.RestartIterator();
		while (m_SceneGraph.Next())
		{
			auto it = m_SceneGraph.GetIterator();
			it->GetData().Transform->CalculateWorldTransformation();
			m_SortSystem->PushRenderData(it->GetData().Renderable, it->GetData().Transform);
		}
		m_SortSystem->SubmitToRenderer();

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
	}
	void EditorLayer::OnEvent(Event& event)
	{
		m_CameraController->OnEvent(event);
	}
	void EditorLayer::OnImGuiRender()
	{
	}
}