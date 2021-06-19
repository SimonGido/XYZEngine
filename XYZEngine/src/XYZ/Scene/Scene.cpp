#include "stdafx.h"
#include "Scene.h"
#include "XYZ/ECS/Serialization/ECSSerializer.h"

#include "XYZ/Core/Input.h"
#include "XYZ/Core/Application.h"
#include "XYZ/Renderer/Renderer.h"
#include "XYZ/Renderer/Renderer2D.h"
#include "XYZ/Renderer/SceneRenderer.h"
#include "XYZ/Renderer/Animation.h"
#include "XYZ/Script/ScriptEngine.h"

#include "SceneEntity.h"

#include <glm/glm.hpp>
#include <glm/gtx/transform.hpp>


namespace XYZ {

	static std::vector<TransformComponent> s_EditTransforms;

	Scene::Scene(const std::string& name)
		:
		m_PhysicsWorld({0.0f, -9.8f}),
		m_PhysicsEntityBuffer(nullptr),
		m_Name(name),
		m_State(SceneState::Edit),
		m_ViewportWidth(0),
		m_ViewportHeight(0)
	{
		m_ECS.ForceStorage<ScriptComponent>();
		m_SceneEntity = m_ECS.CreateEntity();

		m_ECS.EmplaceComponent<Relationship>(m_SceneEntity);
		m_ECS.EmplaceComponent<IDComponent>(m_SceneEntity);
		m_ECS.EmplaceComponent<TransformComponent>(m_SceneEntity);
		m_ECS.EmplaceComponent<SceneTagComponent>(m_SceneEntity, name);	


		m_PhysicsWorld.SetContactListener(&m_ContactListener);
	}

	Scene::~Scene()
	{
		
	}

	SceneEntity Scene::CreateEntity(const std::string& name, const GUID& guid)
	{
		Entity id = m_ECS.CreateEntity();
		SceneEntity entity(id, this);
		IDComponent idComp;
		idComp.ID = guid;

		entity.EmplaceComponent<IDComponent>(guid);
		entity.EmplaceComponent<Relationship>(m_SceneEntity);
		entity.EmplaceComponent<SceneTagComponent>(name);
		entity.EmplaceComponent<TransformComponent>(glm::vec3(0.0f, 0.0f, 0.0f));
		auto& sceneRelation = m_ECS.GetComponent<Relationship>(m_SceneEntity);
		Relationship::SetupRelation(m_SceneEntity, id, m_ECS);

		m_Entities.push_back(id);
		return entity;
	}

	SceneEntity Scene::CreateEntity(const std::string& name, SceneEntity parent, const GUID& guid)
	{
		XYZ_ASSERT(parent.m_Scene == this, "");
		Entity id = m_ECS.CreateEntity();
		SceneEntity entity(id, this);
		IDComponent idComp;
		idComp.ID = guid;

		entity.EmplaceComponent<IDComponent>(guid);
		entity.EmplaceComponent<Relationship>((Entity)entity.m_ID);
		entity.EmplaceComponent<SceneTagComponent>(name);
		entity.EmplaceComponent<TransformComponent>(glm::vec3(0.0f, 0.0f, 0.0f));
		auto& sceneRelation = m_ECS.GetComponent<Relationship>(m_SceneEntity);
		Relationship::SetupRelation(m_SceneEntity, id, m_ECS);

		m_Entities.push_back(id);
		return entity;
	}

	void Scene::DestroyEntity(SceneEntity entity)
	{
		Entity lastEntity = m_Entities.back();
		if (entity.m_ID == m_SelectedEntity)
			m_SelectedEntity = Entity();
		
		for (auto it = m_Entities.begin(); it != m_Entities.end(); ++it)
		{
			if ((*it) == entity.m_ID)
			{
				*it = std::move(m_Entities.back());
				m_Entities.pop_back();
				break;
			}
		}
		Relationship::RemoveRelation(entity.m_ID, m_ECS);
		m_ECS.DestroyEntity(Entity(entity.m_ID));
	}

	void Scene::OnPlay()
	{
		s_EditTransforms.clear();
		s_EditTransforms.resize((size_t)m_ECS.GetHighestID() + 1);
		
		// Find Camera
		m_ECS.ForceStorage<CameraComponent>();
		auto& storage = m_ECS.GetStorage<CameraComponent>();
		if (storage.Size())
		{
			m_CameraEntity = storage.GetEntityAtIndex(0);
			storage.GetComponent(m_CameraEntity).Camera.SetViewportSize(m_ViewportWidth, m_ViewportHeight);
		}
		else
		{
			XYZ_LOG_ERR("No camera found in the scene");
			m_State = SceneState::Edit;
			return;
		}
		// Store original transform components and initialize physics
		for (auto entityID : m_Entities)
		{
			SceneEntity entity(entityID, this);
			TransformComponent& transform = entity.GetComponent<TransformComponent>();
			s_EditTransforms[entityID] = transform;
		}

		setupPhysics();
	
		// Copy stored values to runtime
		auto& scriptStorage = m_ECS.GetStorage<ScriptComponent>();
		for (size_t i = 0; i < scriptStorage.Size(); ++i)
		{
			ScriptComponent& script = scriptStorage.GetComponentAtIndex(i);
			for (auto& field : script.GetFields())
				field.CopyStoredValueToRuntime();
			ScriptEngine::OnCreateEntity({ scriptStorage.GetEntityAtIndex(i), this });
		}
	}

	void Scene::OnStop()
	{
		for (auto entity : m_Entities)
		{
			SceneEntity ent(entity, this);
			ent.GetComponent<TransformComponent>() = s_EditTransforms[(uint32_t)entity];
		}

		auto& rigidStorage = m_ECS.GetStorage<RigidBody2DComponent>();
		for (auto& body : rigidStorage)
		{
			m_PhysicsWorld.DestroyBody(static_cast<b2Body*>(body.RuntimeBody));
		}

		auto& scriptStorage = m_ECS.GetStorage<ScriptComponent>();
		for (size_t i = 0; i < scriptStorage.Size(); ++i)
		{
			ScriptComponent& script = scriptStorage.GetComponentAtIndex(i);
			ScriptEngine::OnDestroyEntity({ scriptStorage.GetEntityAtIndex(i), this });
		}

		delete[]m_PhysicsEntityBuffer;
		m_PhysicsEntityBuffer = nullptr;
	}

	void Scene::OnRender()
	{
		// 3D part here

		///////////////
		SceneEntity cameraEntity(m_CameraEntity, this);
		SceneRendererCamera renderCamera;
		auto& cameraComponent = cameraEntity.GetComponent<CameraComponent>();
		auto& cameraTransform = cameraEntity.GetComponent<TransformComponent>();
		renderCamera.Camera = cameraComponent.Camera;
		renderCamera.ViewMatrix = glm::inverse(cameraTransform.WorldTransform);
		auto [translation, rotation, scale] = cameraTransform.GetWorldComponents();
		renderCamera.ViewPosition = translation;

		SceneRenderer::GetOptions().ShowGrid = false;
		SceneRenderer::BeginScene(this, renderCamera);

		auto renderView = m_ECS.CreateView<TransformComponent, SpriteRenderer>();
		for (auto entity : renderView)
		{
			auto [transform, renderer] = renderView.Get<TransformComponent, SpriteRenderer>(entity);
			SceneRenderer::SubmitSprite(&renderer, &transform);
		}

		auto particleView = m_ECS.CreateView<TransformComponent, ParticleComponent>();
		for (auto entity : particleView)
		{
			auto [transform, particle] = particleView.Get<TransformComponent, ParticleComponent>(entity);
			SceneRenderer::SubmitParticles(&particle, &transform);
		}
		
		auto lightView = m_ECS.CreateView<TransformComponent, PointLight2D>();
		for (auto entity : lightView)
		{
			auto [transform, light] = lightView.Get<TransformComponent, PointLight2D>(entity);
			SceneRenderer::SubmitLight(&light, transform.WorldTransform);
		}
		auto spotLightView = m_ECS.CreateView<TransformComponent, SpotLight2D>();
		for (auto entity : spotLightView)
		{
			auto [transform, light] = spotLightView.Get<TransformComponent, SpotLight2D>(entity);
			SceneRenderer::SubmitLight(&light, transform.WorldTransform);
		}

		SceneRenderer::EndScene();
	}

	void Scene::OnUpdate(Timestep ts)
	{
		int32_t velocityIterations = 6;
		int32_t positionIterations = 2;
		m_PhysicsWorld.Step(ts, velocityIterations, positionIterations);

		m_ECS.ForceStorage<RigidBody2DComponent>();
		auto rigidView = m_ECS.CreateView<TransformComponent, RigidBody2DComponent>();
		for (auto entity : rigidView)
		{
			auto [transform, rigidBody] = rigidView.Get<TransformComponent, RigidBody2DComponent>(entity);
			b2Body* body = static_cast<b2Body*>(rigidBody.RuntimeBody);
			transform.Translation.x = body->GetPosition().x;
			transform.Translation.y = body->GetPosition().y;
			transform.Rotation.z = body->GetAngle();
		}

		auto& scriptStorage = m_ECS.GetStorage<ScriptComponent>();
		for (size_t i = 0; i < scriptStorage.Size(); ++i)
		{
			ScriptComponent& scriptComponent = scriptStorage[i];
			if (!scriptComponent.ModuleName.empty())
				ScriptEngine::OnUpdateEntity({ scriptStorage.GetEntityAtIndex(i),this }, ts);
		}
		
		m_ECS.ForceStorage<AnimatorComponent>();
		auto& animatorStorage = m_ECS.GetStorage<AnimatorComponent>();
		for (auto & anim : animatorStorage)
		{
			anim.Animation->Update(ts);
		}
		
		std::vector<glm::vec4> boxColliders;
		auto boxColliderView = m_ECS.CreateView<TransformComponent, BoxCollider2DComponent>();
		for (auto entity : boxColliderView)
		{
			auto [transform, box] = boxColliderView.Get<TransformComponent, BoxCollider2DComponent>(entity);
			auto [translation, rotation, scale] = transform.GetWorldComponents();
			//boxColliders.push_back({glm::vec4(
			//	translation.x - box.Size.x / 2.0f, translation.y - box.Size.y / 2.0f,
			//	translation.x + box.Size.x / 2.0f, translation.y + box.Size.y / 2.0f
			//)});
		}
		
		auto particleView = m_ECS.CreateView<TransformComponent, ParticleComponent>();
		for (auto entity : particleView)
		{
			auto [transform, particle] = particleView.Get<TransformComponent, ParticleComponent>(entity);
			particle.System->m_BoxColliderStorage->Update(boxColliders.data(), boxColliders.size() * sizeof(glm::vec4));

			particle.ComputeShader->Bind();

			auto& config = particle.System->GetConfiguration();
			particle.ComputeShader->SetMat4("u_Transform", transform.WorldTransform);
			particle.ComputeShader->SetInt("u_NumBoxColliders", boxColliders.size());
			particle.ComputeShader->SetFloat2("u_Force", config.Force);
			// Main module
			particle.ComputeShader->SetInt("u_MainModule.Repeat", (int)config.Repeat);
			particle.ComputeShader->SetInt("u_MainModule.ParticlesEmitted", particle.System->GetEmittedParticles());
			particle.ComputeShader->SetFloat("u_MainModule.LifeTime", 3.0f);
			particle.ComputeShader->SetFloat("u_MainModule.Time", ts);
			particle.ComputeShader->SetFloat("u_MainModule.Speed", config.Speed);
			// Color module
			particle.ComputeShader->SetFloat4("u_ColorModule.StartColor",config.StartColor);
			particle.ComputeShader->SetFloat4("u_ColorModule.EndColor",  config.EndColor);
			// Size module
			particle.ComputeShader->SetFloat2("u_SizeModule.StartSize", config.StartSize);
			particle.ComputeShader->SetFloat2("u_SizeModule.EndSize", config.EndSize);
			// Texture animation module
			particle.ComputeShader->SetInt("u_TextureModule.TilesX", 4);
			particle.ComputeShader->SetInt("u_TextureModule.TilesY", 4);

			particle.System->Update(ts);
			particle.ComputeShader->Compute(32, 32, 1);
		}
		

		updateHierarchy();
	}

	void Scene::OnRenderEditor(const Editor::EditorCamera& camera)
	{
		updateHierarchy();
		SceneRenderer::BeginScene(this, camera.GetViewProjection(), camera.GetPosition());
		
		auto renderView = m_ECS.CreateView<TransformComponent, SpriteRenderer>();
		for (auto entity : renderView)
		{
			auto [transform, renderer] = renderView.Get<TransformComponent, SpriteRenderer>(entity);
			if (renderer.Visible && renderer.SubTexture.Raw() && renderer.Material.Raw())
				SceneRenderer::SubmitSprite(&renderer, &transform);
		}
		auto editorRenderView = m_ECS.CreateView<TransformComponent, EditorSpriteRenderer>();
		for (auto entity : editorRenderView)
		{
			auto [transform, renderer] = editorRenderView.Get<TransformComponent, EditorSpriteRenderer>(entity);
			if (renderer.SubTexture.Raw() && renderer.Material.Raw())
				SceneRenderer::SubmitEditorSprite(&renderer, &transform);
		}
		auto particleView = m_ECS.CreateView<TransformComponent, ParticleComponent>();
		for (auto entity : particleView)
		{
			auto [transform, particle] = particleView.Get<TransformComponent, ParticleComponent>(entity);
			SceneRenderer::SubmitParticles(&particle, &transform);
		}
		
		auto lightView = m_ECS.CreateView<TransformComponent, PointLight2D>();
		for (auto entity : lightView)
		{
			auto [transform, light] = lightView.Get<TransformComponent, PointLight2D>(entity);
			SceneRenderer::SubmitLight(&light, transform.WorldTransform);
		}	

		auto spotLightView = m_ECS.CreateView<TransformComponent, SpotLight2D>();
		for (auto entity : spotLightView)
		{
			auto [transform, light] = spotLightView.Get<TransformComponent, SpotLight2D>(entity);
			SceneRenderer::SubmitLight(&light, transform.WorldTransform);
		}

		if (m_SelectedEntity)
		{
			SceneEntity entity(m_SelectedEntity, this);
			if (entity.HasComponent<CameraComponent>())
			{
				auto& camera = entity.GetComponent<CameraComponent>().Camera;
				camera.SetViewportSize(m_ViewportWidth, m_ViewportHeight);

				auto transformComponent = entity.GetComponent<TransformComponent>();
				auto [translation, rotation, scale] = transformComponent.GetWorldComponents();
				if (camera.GetProjectionType() == CameraProjectionType::Orthographic)
				{
					float size = camera.GetOrthographicProperties().OrthographicSize;
					float aspect = (float)camera.GetViewportWidth() / (float)camera.GetViewportHeight();
					float width = size * aspect;
					float height = size;

					glm::vec3 bottomLeft = { translation.x - width / 2.0f,translation.y - height / 2.0f, translation.z };
					glm::vec3 topRight = { translation.x + width / 2.0f,translation.y + height / 2.0f, translation.z };
					SceneRenderer::SubmitEditorAABB(bottomLeft, topRight, glm::vec4(1.0f));
				}
			}
			else
			{
				SceneRenderer::SubmitEditorAABB(&entity.GetComponent<TransformComponent>(), glm::vec4(1.0f));
			}
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
	SceneEntity Scene::GetEntityByName(const std::string& name)
	{
		return { m_ECS.FindEntity<SceneTagComponent>(name), this };
	}

	SceneEntity Scene::GetSelectedEntity()
	{
		return { m_SelectedEntity, this };
	}


	void Scene::updateHierarchy()
	{
		std::stack<Entity> entities;
		entities.push(m_SceneEntity);
		while (!entities.empty())
		{
			Entity tmp = entities.top();
			entities.pop();
		
			const Relationship& relation = m_ECS.GetComponent<Relationship>(tmp);
			if (relation.NextSibling)
				entities.push(relation.NextSibling);
			if (relation.FirstChild)
				entities.push(relation.FirstChild);
			
			TransformComponent& transform = m_ECS.GetComponent<TransformComponent>(tmp);
			if (relation.Parent)
			{
				TransformComponent& parentTransform = m_ECS.GetComponent<TransformComponent>(relation.Parent);
				transform.WorldTransform = parentTransform.WorldTransform * transform.GetTransform();
			}
			else
			{
				transform.WorldTransform = transform.GetTransform();
			}
		}
	}

	void Scene::setupPhysics()
	{
		auto& storage = m_ECS.GetStorage<RigidBody2DComponent>();
		m_PhysicsEntityBuffer = new SceneEntity[storage.Size()];

		size_t counter = 0;
		for (auto& rigidBody : storage)
		{
			SceneEntity entity(storage.GetEntityAtIndex(counter), this);
			const TransformComponent& transform = entity.GetComponent<TransformComponent>();
			auto [translation, rotation, scale] = transform.GetWorldComponents();

			b2BodyDef bodyDef;
		
			if (rigidBody.Type == RigidBody2DComponent::BodyType::Dynamic)
				bodyDef.type = b2_dynamicBody;
			else if (rigidBody.Type == RigidBody2DComponent::BodyType::Static)
				bodyDef.type = b2_staticBody;
			else if (rigidBody.Type == RigidBody2DComponent::BodyType::Kinematic)
				bodyDef.type = b2_kinematicBody;
			
			
			bodyDef.position.Set(transform.Translation.x, transform.Translation.y);
			bodyDef.angle = transform.Rotation.z;
			bodyDef.userData.pointer = reinterpret_cast<uintptr_t>(&m_PhysicsEntityBuffer[counter]);
			
			b2Body* body = m_PhysicsWorld.CreateBody(&bodyDef);
			rigidBody.RuntimeBody = body;
			
			if (entity.HasComponent<BoxCollider2DComponent>())
			{
				BoxCollider2DComponent& boxCollider = entity.GetComponent<BoxCollider2DComponent>();
				b2PolygonShape poly;
			
				poly.SetAsBox( boxCollider.Size.x / 2.0f, boxCollider.Size.y / 2.0f, 
					   b2Vec2{ boxCollider.Offset.x, boxCollider.Offset.y }, 0.0f);
				b2FixtureDef fixture;
				fixture.shape = &poly;
				fixture.density = boxCollider.Density;
				fixture.friction = boxCollider.Friction;
				boxCollider.RuntimeFixture = body->CreateFixture(&fixture);
			}
			else if (entity.HasComponent<CircleCollider2DComponent>())
			{
				CircleCollider2DComponent& circleCollider = entity.GetComponent<CircleCollider2DComponent>();
				b2CircleShape circle;
				circle.m_radius = circleCollider.Radius;
				circle.m_p = b2Vec2(circleCollider.Offset.x, circleCollider.Offset.y);

				b2FixtureDef fixture;
				fixture.shape = &circle;
				fixture.density  = circleCollider.Density;
				fixture.friction = circleCollider.Friction;
				circleCollider.RuntimeFixture = body->CreateFixture(&fixture);
			}
			else if (entity.HasComponent<PolygonCollider2DComponent>())
			{
				PolygonCollider2DComponent& meshCollider = entity.GetComponent<PolygonCollider2DComponent>();
				b2PolygonShape poly;
				poly.Set((const b2Vec2*)meshCollider.Vertices.data(), meshCollider.Vertices.size());
				b2FixtureDef fixture;
				fixture.shape = &poly;
				fixture.density =  meshCollider.Density;
				fixture.friction = meshCollider.Friction;
				meshCollider.RuntimeFixture = body->CreateFixture(&fixture);
			}
			else if (entity.HasComponent<ChainCollider2DComponent>())
			{
				ChainCollider2DComponent& chainCollider = entity.GetComponent<ChainCollider2DComponent>();
				
				b2ChainShape chain;
				chain.CreateChain((const b2Vec2*)chainCollider.Points.data(), chainCollider.Points.size(),
					{ chainCollider.Points[0].x, chainCollider.Points[0].y },
					{ chainCollider.Points.back().x, chainCollider.Points.back().y });
				b2FixtureDef fixture;
				fixture.shape = &chain;
				fixture.density  = chainCollider.Density;
				fixture.friction = chainCollider.Friction;
				chainCollider.RuntimeFixture = body->CreateFixture(&fixture);
			}
			counter++;
		}

		auto& boxStorage = m_ECS.GetStorage<BoxCollider2DComponent>();
		counter = 0;
		for (auto& box : boxStorage)
		{
			
		}
	}

}