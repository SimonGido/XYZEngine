#include "stdafx.h"
#include "Scene.h"

#include "XYZ/Core/Input.h"
#include "XYZ/Core/Application.h"
#include "XYZ/Renderer/Renderer.h"
#include "XYZ/Renderer/Renderer2D.h"
#include "XYZ/Renderer/SceneRenderer.h"
#include "XYZ/ECS/ComponentGroup.h"
#include "XYZ/Script/ScriptEngine.h"
#include "XYZ/Physics/ContactListener.h"
#include "XYZ/Renderer/PhysicsDebugRenderer.h"

#include "SceneEntity.h"


#include <box2d/box2d.h>

#include <glm/glm.hpp>
#include <glm/gtx/transform.hpp>


namespace XYZ {

	static ContactListener s_ContactListener;
	static PhysicsDebugRenderer s_DebugRenderer;

	Scene::Scene(const std::string& name)
		:
		m_Name(name),
		m_SelectedEntity(NULL_ENTITY),
		m_CameraEntity(NULL_ENTITY)
	{
		m_ViewportWidth = 0;
		m_ViewportHeight = 0;

		m_CameraTexture = Texture2D::Create({ TextureWrap::Clamp, TextureParam::Linear, TextureParam::Nearest }, "Assets/Textures/Gui/Camera.png");
		m_CameraSubTexture = Ref<SubTexture>::Create(m_CameraTexture, glm::vec4(0.0f, 0.0f, 1.0f, 1.0f));
		m_CameraMaterial = Ref<Material>::Create(Shader::Create("Assets/Shaders/DefaultShader.glsl"));
		m_CameraMaterial->Set("u_Color", glm::vec4(1.0f));
		m_CameraMaterial->Set("u_Texture", m_CameraTexture);
		m_CameraRenderer = SpriteRenderer(m_CameraMaterial, m_CameraSubTexture, glm::vec4(1.0f), 0);

		m_RenderView = &m_ECS.CreateView<TransformComponent, SpriteRenderer>();
		m_ParticleView = &m_ECS.CreateView<TransformComponent, ParticleComponent>();
		m_LightView = &m_ECS.CreateView<TransformComponent, PointLight2D>();
		m_AnimatorView = &m_ECS.CreateView<AnimatorComponent>();
		m_RigidBodyView = &m_ECS.CreateView<TransformComponent, RigidBody2DComponent>();

		m_ECS.ForceStorage<ScriptComponent>();
		m_ScriptStorage = m_ECS.GetStorage<ScriptComponent>();
		m_AnimatorStorage = m_ECS.GetStorage<AnimatorComponent>();

		m_PhysicsWorld = new b2World(b2Vec2{ 0.0f,-9.8f });
		m_PhysicsWorld->SetContactListener(&s_ContactListener);
		m_PhysicsWorld->SetDebugDraw(&s_DebugRenderer);
		s_DebugRenderer.SetFlags(b2Draw::e_shapeBit | b2Draw::e_aabbBit | b2Draw::e_centerOfMassBit);
	}

	Scene::~Scene()
	{
		delete m_PhysicsWorld;
	}

	SceneEntity Scene::CreateEntity(const std::string& name, const GUID& guid)
	{
		SceneEntity entity(m_ECS.CreateEntity(), this);
		IDComponent id;
		id.ID = guid;
		entity.AddComponent<IDComponent>(id);
		entity.AddComponent<Relationship>(Relationship());
		entity.AddComponent<SceneTagComponent>(SceneTagComponent(name));
		entity.AddComponent<TransformComponent>(TransformComponent(glm::vec3(0.0f, 0.0f, 0.0f)));


		m_Entities.push_back(entity);
		return entity;
	}

	void Scene::DestroyEntity(SceneEntity entity)
	{
		uint32_t lastEntity = m_Entities.back();
		if (entity.m_ID == m_SelectedEntity)
			m_SelectedEntity = NULL_ENTITY;
		// Swap with last and delete
		auto it = std::find(m_Entities.begin(), m_Entities.end(), (uint32_t)entity);
		if (it != m_Entities.end())
		{
			*it = std::move(m_Entities.back());
			m_Entities.pop_back();
		}
		m_ECS.DestroyEntity(entity);
	}


	void Scene::OnPlay()
	{
		for (auto entity : m_Entities)
		{
			Entity ent(entity, &m_ECS);
			if (ent.HasComponent<CameraComponent>())
			{
				ent.GetStorageComponent<CameraComponent>().Camera.SetViewportSize(m_ViewportWidth, m_ViewportHeight);
				m_CameraEntity = ent;
				break;
			}
		}

		for (size_t i = 0; i < m_ScriptStorage->Size(); ++i)
		{

		}

		uint32_t counter = 0;
		m_PhysicsBodyEntityBuffer = new SceneEntity[m_Entities.size()];
		for (auto entityID : m_Entities)
		{
			SceneEntity entity(entityID, this);
			if (!entity.HasComponent<BoxCollider2DComponent>() && !entity.HasComponent<CameraComponent>())
			{
				SceneTagComponent& sceneTag = entity.GetComponent<SceneTagComponent>();
				
				auto& boxCollider = entity.AddComponent<BoxCollider2DComponent>({});
				auto& rigidBody = entity.AddComponent<RigidBody2DComponent>({});
				auto& transform = entity.GetComponent<TransformComponent>();
				if (sceneTag.Name == "Background")
				{
					rigidBody.BodyType = RigidBody2DComponent::Type::Static;
				}
				b2BodyDef bodyDef;
				SceneEntity* entityStorage = &m_PhysicsBodyEntityBuffer[counter++];
				*entityStorage = SceneEntity(entityID, this);

				if (rigidBody.BodyType == RigidBody2DComponent::Type::Dynamic)
					bodyDef.type = b2_dynamicBody;
				else if (rigidBody.BodyType == RigidBody2DComponent::Type::Static)
					bodyDef.type = b2_staticBody;
				else if (rigidBody.BodyType == RigidBody2DComponent::Type::Kinematic)
					bodyDef.type = b2_kinematicBody;
				bodyDef.position.Set(transform.Translation.x, transform.Translation.y);
				bodyDef.angle = transform.Rotation.z;
				bodyDef.userData.pointer = reinterpret_cast<uintptr_t>(entityStorage);
				b2Body * body = m_PhysicsWorld->CreateBody(&bodyDef);
				rigidBody.RuntimeBody = body;

				b2PolygonShape shape;
				boxCollider.Size = transform.Scale / 2.0f;
				shape.SetAsBox(boxCollider.Size.x, boxCollider.Size.y);
				b2FixtureDef fixture;
				fixture.shape = &shape;
				fixture.density = boxCollider.Density;
				fixture.friction = boxCollider.Friction;
				boxCollider.RuntimeFixture = body->CreateFixture(&fixture);
			}
		}

	}

	void Scene::OnEvent(Event& e)
	{
	}

	void Scene::OnStop()
	{
		delete[] m_PhysicsBodyEntityBuffer;
	}

	void Scene::OnRender()
	{
		// 3D part here

		///////////////
		Entity cameraEntity(m_CameraEntity, &m_ECS);
		SceneRendererCamera renderCamera;
		auto& cameraComponent = cameraEntity.GetStorageComponent<CameraComponent>();
		auto& cameraTransform = cameraEntity.GetStorageComponent<TransformComponent>();
		renderCamera.Camera = cameraComponent.Camera;
		renderCamera.ViewMatrix = glm::inverse(cameraTransform.GetTransform());

		SceneRenderer::GetOptions().ShowGrid = false;
		SceneRenderer::BeginScene(this, renderCamera);
		for (size_t i = 0; i < m_RenderView->Size(); ++i)
		{
			auto [transform, renderer] = (*m_RenderView)[i];
			SceneRenderer::SubmitSprite(&renderer, &transform);
		}
		for (size_t i = 0; i < m_ParticleView->Size(); ++i)
		{
			auto [transform, particle] = (*m_ParticleView)[i];
			SceneRenderer::SubmitParticles(&particle, &transform);
		}

		for (size_t i = 0; i < m_LightView->Size(); ++i)
		{
			auto [transform, light] = (*m_LightView)[i];
			SceneRenderer::SubmitLight(&light, transform.GetTransform());
		}
		SceneRenderer::EndScene();
	}

	void Scene::OnUpdate(Timestep ts)
	{
		int32_t velocityIterations = 6;
		int32_t positionIterations = 2;
		m_PhysicsWorld->Step(ts, velocityIterations, positionIterations);
		m_PhysicsWorld->DebugDraw();

		for (size_t i = 0; i < m_RigidBodyView->Size(); ++i)
		{
			auto [transform, rigidBody] = (*m_RigidBodyView)[i];
			b2Body* body = static_cast<b2Body*>(rigidBody.RuntimeBody);
			transform.Translation.x = body->GetPosition().x;
			transform.Translation.y = body->GetPosition().y;
			transform.Rotation.z = body->GetAngle();
		}
		for (size_t i = 0; i < m_ScriptStorage->Size(); ++i)
		{
			ScriptComponent& scriptComponent = (*m_ScriptStorage)[i];
			ScriptEngine::OnUpdateEntity({ m_ScriptStorage->GetEntityAtIndex(i),this }, ts);
		}

		for (size_t i = 0; i < m_AnimatorStorage->Size(); ++i)
		{
			AnimatorComponent& anim = (*m_AnimatorStorage)[i];
			anim.Controller.Update(ts);
		}

		for (size_t i = 0; i < m_AnimatorView->Size(); ++i)
		{
			auto [animator] = (*m_AnimatorView)[i];
			animator.Controller.Update(ts);
		}

		for (size_t i = 0; i < m_ParticleView->Size(); ++i)
		{
			auto [transform, particle] = (*m_ParticleView)[i];
			auto material = particle.ComputeMaterial->GetParentMaterial();
			auto materialInstance = particle.ComputeMaterial;

			materialInstance->Set("u_Time", ts);
			materialInstance->Set("u_ParticlesInExistence", (int)std::ceil(particle.ParticleEffect->GetEmittedParticles()));

			material->GetShader()->Bind();
			materialInstance->Bind();

			particle.ParticleEffect->Update(ts);
			material->GetShader()->Compute(32, 32, 1);
		}
	}

	void Scene::OnRenderEditor(const EditorCamera& camera)
	{
		SceneRenderer::BeginScene(this, camera.GetViewProjection());
		if (m_SelectedEntity != NULL_ENTITY)
		{
			if (m_ECS.Contains<CameraComponent>(m_SelectedEntity))
			{
				SceneRenderer::SubmitSprite(&m_CameraRenderer, &m_ECS.GetComponent<TransformComponent>(m_SelectedEntity));
				showCamera(m_SelectedEntity);
			}
			else
				showSelection(m_SelectedEntity);
		}
		for (size_t i = 0; i < m_RenderView->Size(); ++i)
		{
			auto [transform, renderer] = (*m_RenderView)[i];
			if (renderer.IsVisible && renderer.SubTexture.Raw())
				SceneRenderer::SubmitSprite(&renderer, &transform);
		}
		for (size_t i = 0; i < m_ParticleView->Size(); ++i)
		{
			auto [transform, particle] = (*m_ParticleView)[i];
			SceneRenderer::SubmitParticles(&particle, &transform);
		}

		for (size_t i = 0; i < m_LightView->Size(); ++i)
		{
			auto [transform, light] = (*m_LightView)[i];
			SceneRenderer::SubmitLight(&light, transform.GetTransform());
		}


		SceneRenderer::EndScene();
	}

	void Scene::SetViewportSize(uint32_t width, uint32_t height)
	{
		m_ViewportWidth = width;
		m_ViewportHeight = height;
	}

	SceneEntity Scene::GetEntity(uint32_t index)
	{
		return { m_Entities[index], this };
	}

	SceneEntity Scene::GetSelectedEntity()
	{
		return { m_SelectedEntity, this };
	}

	void Scene::showSelection(uint32_t entity)
	{
		auto transformComponent = m_ECS.GetComponent<TransformComponent>(entity);
		auto& translation = transformComponent.Translation;
		auto& scale = transformComponent.Scale;

		glm::vec3 topLeft = { translation.x - scale.x / 2,translation.y + scale.y / 2, translation.z };
		glm::vec3 topRight = { translation.x + scale.x / 2,translation.y + scale.y / 2, translation.z };
		glm::vec3 bottomLeft = { translation.x - scale.x / 2,translation.y - scale.y / 2, translation.z };
		glm::vec3 bottomRight = { translation.x + scale.x / 2,translation.y - scale.y / 2, translation.z };
	
		Renderer2D::SubmitLine(topLeft, topRight);
		Renderer2D::SubmitLine(topRight, bottomRight);
		Renderer2D::SubmitLine(bottomRight, bottomLeft);
		Renderer2D::SubmitLine(bottomLeft, topLeft);
	}

	void Scene::showCamera(uint32_t entity)
	{
		auto& camera = m_ECS.GetComponent<CameraComponent>(entity).Camera;
		auto transformComponent = m_ECS.GetComponent<TransformComponent>(entity);

		auto& translation = transformComponent.Translation;
		if (camera.GetProjectionType() == CameraProjectionType::Orthographic)
		{
			float size = camera.GetOrthographicProperties().OrthographicSize;
			float aspect = (float)m_ViewportWidth / (float)m_ViewportHeight;
			float width = size * aspect;
			float height = size;

			glm::vec3 topLeft = { translation.x - width / 2.0f,translation.y + height / 2.0f,1.0f };
			glm::vec3 topRight = { translation.x + width / 2.0f,translation.y + height / 2.0f,1.0f };
			glm::vec3 bottomLeft = { translation.x - width / 2.0f,translation.y - height / 2.0f,1.0f };
			glm::vec3 bottomRight = { translation.x + width / 2.0f,translation.y - height / 2.0f,1.0f };

			Renderer2D::SubmitLine(topLeft, topRight);
			Renderer2D::SubmitLine(topRight, bottomRight);
			Renderer2D::SubmitLine(bottomRight, bottomLeft);
			Renderer2D::SubmitLine(bottomLeft, topLeft);
		}
	}

}