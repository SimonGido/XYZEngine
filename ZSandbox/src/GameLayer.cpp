#include "GameLayer.h"

#include <random>

#include "imgui/imgui.h"
#include <imgui/examples/imgui_impl_glfw.h>
#include <imgui/examples/imgui_impl_opengl3.h>

GameLayer::GameLayer()
{
}

GameLayer::~GameLayer()
{
}

void GameLayer::OnAttach()
{
	XYZ::Audio::Init();
	m_RendererSystem = std::make_shared<XYZ::SpriteRenderer>();
	m_PhysicsSystem = XYZ::ECSManager::Get().RegisterSystem<XYZ::PhysicsSystem>();
	m_InterSystem = XYZ::ECSManager::Get().RegisterSystem<XYZ::InterpolatedMovementSystem>();
	m_GridCollisionSystem = XYZ::ECSManager::Get().RegisterSystem<XYZ::GridCollisionSystem>();
	m_GridCollisionSystem->ResizeGrid(21, 21, 1, 0, 0);
	m_RealGridCollisionSystem = XYZ::ECSManager::Get().RegisterSystem<XYZ::RealGridCollisionSystem>();
	m_RealGridCollisionSystem->CreateGrid(21, 21, 1);

	m_ParticleSystem = XYZ::ECSManager::Get().RegisterSystem<XYZ::ParticleSystem2D>();
	m_SpriteSystem = XYZ::ECSManager::Get().RegisterSystem<XYZ::SpriteSystem>();

	XYZ::Renderer::Init();


	m_CameraController = std::make_shared<XYZ::OrthoCameraController>(16.0f / 8.0f, false);

	m_Material = XYZ::Material::Create(XYZ::Shader::Create("TextureShader", "Assets/Shaders/DefaultShader.glsl"));
	m_Material->Set("u_Texture", XYZ::Texture2D::Create(XYZ::TextureWrap::Clamp, "Assets/Textures/bomb.png"), 0);
	m_Material->Set("u_Texture", XYZ::Texture2D::Create(XYZ::TextureWrap::Clamp, "Assets/Textures/wall.png"), 1);
	m_Material->Set("u_Texture", XYZ::Texture2D::Create(XYZ::TextureWrap::Repeat, "Assets/Textures/background.png"), 2);



	std::shared_ptr<XYZ::Texture2D> texture = XYZ::Texture2D::Create(XYZ::TextureWrap::Clamp, "Assets/Textures/player_sprite.png");
	m_Material->Set("u_Texture", texture, 3);
	m_Material->Set("u_ViewProjection", m_CameraController->GetCamera().GetViewProjectionMatrix());
	m_Material->SetFlags(XYZ::RenderFlags::TransparentFlag);




	m_ParticleEntity = XYZ::ECSManager::Get().CreateEntity();
	InitBackgroundParticles(m_ParticleEntity);

	//m_Audio = XYZ::Audio::Create("Assets/Audio/sample.mp3");
	//// Audio setup
	//m_Audio->SetLoop(true);
	//m_Audio->SetPosition(glm::vec2(0));
	//m_Audio->SetGain(5.0f);
	//// Play whole track on different thread i guess
	//m_Audio->Play();


	

	m_World = XYZ::ECSManager::Get().CreateEntity();
	XYZ::ECSManager::Get().AddComponent<XYZ::Transform2D>(m_World, XYZ::Transform2D{
		glm::vec3(0,0,0)
		});

	m_Player = XYZ::ECSManager::Get().CreateEntity();
	XYZ::ECSManager::Get().AddComponent<XYZ::Renderable2D>(m_Player, XYZ::Renderable2D{
		m_Material,
		std::make_shared<XYZ::SubTexture2D>(texture,glm::vec2(0,0),glm::vec2(texture->GetWidth()/8,texture->GetHeight()/3)),
		glm::vec4(1,1,1,1),
		3,
		true
		});

	XYZ::ECSManager::Get().AddComponent<XYZ::Transform2D>(m_Player, XYZ::Transform2D{
		glm::vec3(0,0,0)
		});

	m_PlayerChild = XYZ::ECSManager::Get().CreateEntity();
	XYZ::ECSManager::Get().AddComponent<XYZ::Renderable2D>(m_PlayerChild, XYZ::Renderable2D{
		m_Material,
		std::make_shared<XYZ::SubTexture2D>(texture,glm::vec2(0,0),glm::vec2(texture->GetWidth() / 8,texture->GetHeight() / 3)),
		glm::vec4(1,1,1,1),
		3,
		true
		});

	XYZ::ECSManager::Get().AddComponent<XYZ::Transform2D>(m_PlayerChild, XYZ::Transform2D{
		glm::vec3(3,3,0)
		});

	m_PlayerChild2 = XYZ::ECSManager::Get().CreateEntity();
	XYZ::ECSManager::Get().AddComponent<XYZ::Renderable2D>(m_PlayerChild2, XYZ::Renderable2D{
		m_Material,
		std::make_shared<XYZ::SubTexture2D>(texture,glm::vec2(0,0),glm::vec2(texture->GetWidth() / 8,texture->GetHeight() / 3)),
		glm::vec4(1,1,1,1),
		3,
		true
		});

	XYZ::ECSManager::Get().AddComponent<XYZ::Transform2D>(m_PlayerChild2, XYZ::Transform2D{
		glm::vec3(3,3,0)
		});


	m_FrameBuffer = XYZ::FrameBuffer::Create(XYZ::FrameBufferSpecs{ 100,100 });
	m_FrameBuffer->CreateColorAttachment(XYZ::FrameBufferFormat::RGBA16F);
	m_FrameBuffer->CreateColorAttachment(XYZ::FrameBufferFormat::RGBA16F);
	m_FrameBuffer->CreateColorAttachment(XYZ::FrameBufferFormat::RGBA16F);
	m_FrameBuffer->CreateColorAttachment(XYZ::FrameBufferFormat::RGBA16F);
	m_FrameBuffer->Resize();

	m_WorldTransform = XYZ::ECSManager::Get().GetComponent<XYZ::Transform2D>(m_World);
	m_PlayerTransform = XYZ::ECSManager::Get().GetComponent<XYZ::Transform2D>(m_Player);
	m_PlayerChildTransform = XYZ::ECSManager::Get().GetComponent<XYZ::Transform2D>(m_PlayerChild);
	m_PlayerChildTransform2 = XYZ::ECSManager::Get().GetComponent<XYZ::Transform2D>(m_PlayerChild2);

	m_PlayerRenderable = XYZ::ECSManager::Get().GetComponent<XYZ::Renderable2D>(m_Player);
	m_PlayerChildRenderable = XYZ::ECSManager::Get().GetComponent<XYZ::Renderable2D>(m_PlayerChild);
	m_PlayerChildRenderable2 = XYZ::ECSManager::Get().GetComponent<XYZ::Renderable2D>(m_PlayerChild2);
	
	m_SceneTree.InsertNode(XYZ::Node<SceneObject>({ nullptr, m_WorldTransform }));
	m_SceneTree.InsertNode(XYZ::Node<SceneObject>({ m_PlayerRenderable,m_PlayerTransform }));
	m_SceneTree.InsertNode(XYZ::Node<SceneObject>({ m_PlayerChildRenderable,m_PlayerChildTransform }));
	m_SceneTree.InsertNode(XYZ::Node<SceneObject>({ m_PlayerChildRenderable2,m_PlayerChildTransform2 }));

	m_SceneTree.SetRoot(0);
	m_SceneTree.SetParent(0, 1, SceneSetup());
	m_SceneTree.SetParent(1, 2, SceneSetup());
	m_SceneTree.SetParent(2, 3, SceneSetup());
	
	m_Propagation.RendererSystem = m_RendererSystem;
}

void GameLayer::OnDetach()
{
	XYZ::Renderer2D::Shutdown();
}

void GameLayer::OnImGuiRender()
{
	
}

void GameLayer::OnUpdate(float dt)
{

	if (XYZ::Input::IsKeyPressed(XYZ::KeyCode::XYZ_KEY_UP))
	{
		m_PlayerTransform->Translate(glm::vec3(0, 0.005, 0));
	}
	else if (XYZ::Input::IsKeyPressed(XYZ::KeyCode::XYZ_KEY_DOWN))
	{
		m_PlayerTransform->Translate(glm::vec3(0, -0.005, 0));
	}

	if (XYZ::Input::IsKeyPressed(XYZ::KeyCode::XYZ_KEY_LEFT))
	{
		m_PlayerTransform->Translate(glm::vec3(-0.005, 0, 0));
	}
	else if (XYZ::Input::IsKeyPressed(XYZ::KeyCode::XYZ_KEY_RIGHT))
	{
		m_PlayerTransform->Translate(glm::vec3(0.005, 0, 0));
	}

	if (XYZ::Input::IsKeyPressed(XYZ::KeyCode::XYZ_KEY_1))
	{
		m_PlayerTransform->Rotate(0.05);
	}
	else if (XYZ::Input::IsKeyPressed(XYZ::KeyCode::XYZ_KEY_2))
	{
		m_PlayerTransform->Rotate(-0.05);
	}



	if (XYZ::Input::IsKeyPressed(XYZ::KeyCode::XYZ_KEY_I))
	{
		m_PlayerChildTransform->Translate(glm::vec3(0, 0.005, 0));
	}
	else if (XYZ::Input::IsKeyPressed(XYZ::KeyCode::XYZ_KEY_K))
	{
		m_PlayerChildTransform->Translate(glm::vec3(0, -0.005, 0));
	}

	if (XYZ::Input::IsKeyPressed(XYZ::KeyCode::XYZ_KEY_J))
	{
		m_PlayerChildTransform->Translate(glm::vec3(-0.005, 0, 0));
	}
	else if (XYZ::Input::IsKeyPressed(XYZ::KeyCode::XYZ_KEY_L))
	{
		m_PlayerChildTransform->Translate(glm::vec3(0.005, 0, 0));
	}

	if (XYZ::Input::IsKeyPressed(XYZ::KeyCode::XYZ_KEY_3))
	{
		m_PlayerChildTransform->Rotate(0.05);
	}
	else if (XYZ::Input::IsKeyPressed(XYZ::KeyCode::XYZ_KEY_4))
	{
		m_PlayerChildTransform->Rotate(-0.05);
	}

	m_SceneTree.Propagate(m_Propagation, 0);

	

	XYZ::RenderCommand::Clear();
	XYZ::RenderCommand::SetClearColor(glm::vec4(0.2, 0.2, 0.5, 1));
	m_RendererSystem->SubmitToRenderer();

	
	m_CameraController->OnUpdate(dt);

	m_RealGridCollisionSystem->Update(dt);
	m_GridCollisionSystem->Update(dt);
	m_PhysicsSystem->Update(dt);
	m_InterSystem->Update(dt);

	m_ParticleSystem->Update(dt);
	m_SpriteSystem->Update(dt);

	m_Material->Set("u_ViewProjection", m_CameraController->GetCamera().GetViewProjectionMatrix());
	m_ParticleMaterial->Set("u_ViewProjection", m_CameraController->GetCamera().GetViewProjectionMatrix());
		

}

void GameLayer::OnEvent(XYZ::Event& event)
{
	m_CameraController->OnEvent(event);
}

void GameLayer::InitBackgroundParticles(XYZ::Entity entity)
{
	int count = 100;
	m_ParticleMaterial = XYZ::Material::Create(XYZ::Shader::Create("ParticleShader", "Assets/Shaders/Particle/ParticleShader.glsl"));
	m_ParticleMaterial->Set("u_Texture", XYZ::Texture2D::Create(XYZ::TextureWrap::Clamp, "Assets/Textures/bubble.png"));
	m_ParticleMaterial->SetFlags(XYZ::RenderFlags::InstancedFlag);

	m_ParticleMaterial->GetShader()->AddSource("Assets/Shaders/Variables/PredefinedVariables.glsl");
	auto material = XYZ::Material::Create(XYZ::Shader::Create("Assets/Shaders/Particle/ParticleComputeShader.glsl"));

	XYZ::ECSManager::Get().AddComponent(entity, XYZ::ParticleEffect2D(count, material, m_ParticleMaterial));
	auto effect = XYZ::ECSManager::Get().GetComponent<XYZ::ParticleEffect2D>(entity);


	std::random_device rd;
	std::mt19937 rng(rd());
	std::uniform_real_distribution<> dist(-1, 1);

	XYZ::ParticleVertex* m_Vertices = new XYZ::ParticleVertex[count];
	XYZ::ParticleInformation* m_Data = new XYZ::ParticleInformation[count];

	for (int i = 0; i < count; ++i)
	{
		m_Vertices[i].Position = glm::vec4(((float)1/(float)count)*i, 0.0f, 0.0f, 1.0f);
		m_Vertices[i].Color = glm::vec4(0, 1, 1, 1);
		m_Vertices[i].Rotation = 0.0f;
		m_Vertices[i].TexCoordOffset = glm::vec2(0);

		m_Data[i].DefaultPosition.x = m_Vertices[i].Position.x;
		m_Data[i].DefaultPosition.y = m_Vertices[i].Position.y;
		m_Data[i].ColorBegin = m_Vertices[i].Color;
		m_Data[i].ColorEnd = m_Vertices[i].Color;
		m_Data[i].SizeBegin = 0.2f;
		m_Data[i].SizeEnd = 0.2f;
		m_Data[i].Rotation = dist(rng) * 50;
		m_Data[i].Velocity = glm::vec2(dist(rng), 1.0f);
		m_Data[i].DefaultVelocity = m_Data[i].Velocity;
		m_Data[i].LifeTime = fabs(dist(rng)) * 1 + 3;
	}
	effect->SetParticlesRange(m_Vertices, m_Data, 0, count);
}