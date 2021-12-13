#include "stdafx.h"
#include "Scene.h"

#include "XYZ/Core/Input.h"
#include "XYZ/Core/Application.h"

#include "XYZ/Renderer/Renderer.h"
#include "XYZ/Renderer/Renderer2D.h"
#include "XYZ/Renderer/SceneRenderer.h"
#include "XYZ/Renderer/Renderer2D.h"

#include "XYZ/Animation/Animator.h"

#include "XYZ/Script/ScriptEngine.h"
#include "EditorComponents.h"
#include "Components.h"
#include "SceneEntity.h"

#include "XYZ/Debug/Profiler.h"

#include <glm/glm.hpp>
#include <glm/gtx/transform.hpp>


namespace XYZ {

	static std::vector<TransformComponent> s_EditTransforms;

	Scene::Scene(const std::string& name)
		:
		m_PhysicsWorld({ 0.0f, -9.8f }),
		m_PhysicsEntityBuffer(nullptr),
		m_Name(name),
		m_State(SceneState::Edit),
		m_ViewportWidth(0),
		m_ViewportHeight(0)
	{
		m_ECS.CreateStorage<ScriptComponent>();
		m_SceneEntity = m_ECS.CreateEntity();

		m_ECS.EmplaceComponent<Relationship>(m_SceneEntity);
		m_ECS.EmplaceComponent<IDComponent>(m_SceneEntity);
		m_ECS.EmplaceComponent<TransformComponent>(m_SceneEntity);
		m_ECS.EmplaceComponent<SceneTagComponent>(m_SceneEntity, name);	

		ScopedLock<b2World> physicsWorld = m_PhysicsWorld.GetWorld();
		physicsWorld->SetContactListener(&m_ContactListener);
	}

	Scene::~Scene()
	{
	}

	SceneEntity Scene::CreateEntity(const std::string& name, const GUID& guid)
	{
		const Entity id = m_ECS.CreateEntity();
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
		const Entity id = m_ECS.CreateEntity();
		SceneEntity entity(id, this);
		IDComponent idComp;
		idComp.ID = guid;

		entity.EmplaceComponent<IDComponent>(guid);
		entity.EmplaceComponent<Relationship>((Entity)entity.m_ID);
		entity.EmplaceComponent<SceneTagComponent>(name);
		entity.EmplaceComponent<TransformComponent>(glm::vec3(0.0f, 0.0f, 0.0f));
		auto& sceneRelation = m_ECS.GetComponent<Relationship>(m_SceneEntity);
		Relationship::SetupRelation(parent, id, m_ECS);

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
		m_ECS.CreateStorage<CameraComponent>();
		auto& storage = m_ECS.GetStorage<CameraComponent>();
		if (storage.Size())
		{
			m_CameraEntity = storage.GetEntityAtIndex(0);
			storage.GetComponent(m_CameraEntity).Camera.SetViewportSize(m_ViewportWidth, m_ViewportHeight);
		}
		else
		{
			XYZ_CORE_ERROR("No camera found in the scene");
			m_State = SceneState::Edit;
			return;
		}
		// Store original transform components and initialize physics
		for (auto entityID : m_Entities)
		{
			SceneEntity entity(entityID, this);
			const TransformComponent& transform = entity.GetComponent<TransformComponent>();
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

		auto& storageParticleCPU = m_ECS.GetStorage<ParticleComponentCPU>();
		for (auto &it : storageParticleCPU)
		{
			it.System.Reset();
		}
	}

	void Scene::OnStop()
	{
		for (auto entity : m_Entities)
		{
			SceneEntity ent(entity, this);
			ent.GetComponent<TransformComponent>() = s_EditTransforms[(uint32_t)entity];
		}
		{
			ScopedLock<b2World> physicsWorld = m_PhysicsWorld.GetWorld();
			auto& rigidStorage = m_ECS.GetStorage<RigidBody2DComponent>();
			for (const auto& body : rigidStorage)
			{
				physicsWorld->DestroyBody(static_cast<b2Body*>(body.RuntimeBody));
			}
		}
		auto& scriptStorage = m_ECS.GetStorage<ScriptComponent>();
		for (size_t i = 0; i < scriptStorage.Size(); ++i)
		{
			ScriptComponent& script = scriptStorage.GetComponentAtIndex(i);
			ScriptEngine::OnDestroyEntity({ scriptStorage.GetEntityAtIndex(i), this });
		}

		auto& storageParticleCPU = m_ECS.GetStorage<ParticleComponentCPU>();
		for (auto& it : storageParticleCPU)
		{
			it.System.Reset();
		}

		delete[]m_PhysicsEntityBuffer;
		m_PhysicsEntityBuffer = nullptr;
	}

	void Scene::OnUpdate(Timestep ts)
	{
		XYZ_PROFILE_FUNC("Scene::OnUpdate");
		m_PhysicsWorld.Step(ts);

		m_ECS.CreateStorage<RigidBody2DComponent>();
		auto rigidView = m_ECS.CreateView<TransformComponent, RigidBody2DComponent>();
		for (const auto entity : rigidView)
		{
			auto [transform, rigidBody] = rigidView.Get<TransformComponent, RigidBody2DComponent>(entity);
			const b2Body* body = static_cast<b2Body*>(rigidBody.RuntimeBody);
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

		m_ECS.CreateStorage<AnimatorComponent>();
		auto& animatorStorage = m_ECS.GetStorage<AnimatorComponent>();
		for (auto& anim : animatorStorage)
		{
			anim.Animator->Update(ts);
		}

		auto particleViewCPU = m_ECS.CreateView<TransformComponent, ParticleComponentCPU>();
		for (const auto entity : particleViewCPU)
		{
			auto [transform, particle] = particleViewCPU.Get<TransformComponent, ParticleComponentCPU>(entity);
			auto& system = particle.System;
			system.Update(ts);
		}
		// TODO: This will be called only from script i guess
		auto particleView = m_ECS.CreateView<TransformComponent, ParticleComponentGPU>();
		for (const auto entity : particleView)
		{
			auto [transform, particle] = particleView.Get<TransformComponent, ParticleComponentGPU>(entity);
			auto& particleMaterial = particle.System->m_Renderer->m_ParticleMaterial;
			particleMaterial->Set("u_MaxParticles", particleMaterial->GetMaxParticles());
			particleMaterial->Set("u_MainModule.Time", ts);
			particleMaterial->Set("u_MainModule.ParticlesEmitted", (int)particle.System->GetEmittedParticles());
			auto& computeShader = particleMaterial->GetComputeShader();
			computeShader->Bind();
			particle.System->Update(ts);
			particleMaterial->Compute();
		}

		updateHierarchy();
	}

	void Scene::OnRender(Ref<SceneRenderer> sceneRenderer)
	{
		XYZ_PROFILE_FUNC("Scene::OnRender");
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

		sceneRenderer->GetOptions().ShowGrid = false;
		sceneRenderer->SetViewportSize(m_ViewportWidth, m_ViewportHeight);
		sceneRenderer->BeginScene(renderCamera);

		auto renderView = m_ECS.CreateView<TransformComponent, SpriteRenderer>();
		for (auto entity : renderView)
		{
			auto [transform, renderer] = renderView.Get<TransformComponent, SpriteRenderer>(entity);
			if (renderer.Visible && renderer.SubTexture.Raw() && renderer.Material.Raw())
				sceneRenderer->SubmitSprite(
				renderer.Material, renderer.SubTexture, renderer.SortLayer, renderer.Color, transform.WorldTransform
			);
		}

		//auto particleView = m_ECS.CreateView<TransformComponent, ParticleComponentGPU>();
		//for (auto entity : particleView)
		//{
		//	auto [transform, particle] = particleView.Get<TransformComponent, ParticleComponentGPU>(entity);
		//	sceneRenderer->SubmitRendererCommand(particle.System->m_Renderer, transform.WorldTransform);
		//}
		//

		auto particleViewCPU = m_ECS.CreateView<TransformComponent, ParticleComponentCPU, MeshComponent>();
		for (auto entity : particleViewCPU)
		{
			auto [transform, particle, mesh] = particleViewCPU.Get<TransformComponent, ParticleComponentCPU, MeshComponent>(entity);
			particle.System.SetupForRender(sceneRenderer);
			auto renderData = particle.System.GetRenderDataRead();
			mesh.Mesh->SetVertexBufferData(1, renderData->m_RenderParticleData, renderData->m_RenderParticleData.SizeInBytes());
			sceneRenderer->SubmitMeshInstanced(mesh.Mesh, transform.WorldTransform, renderData->m_InstanceCount);
		}
		
		//auto lightView = m_ECS.CreateView<TransformComponent, PointLight2D>();
		//for (auto entity : lightView)
		//{
		//	auto [transform, light] = lightView.Get<TransformComponent, PointLight2D>(entity);
		//	sceneRenderer->SubmitLight(light, transform.WorldTransform);
		//}
		//auto spotLightView = m_ECS.CreateView<TransformComponent, SpotLight2D>();
		//for (auto entity : spotLightView)
		//{
		//	auto [transform, light] = spotLightView.Get<TransformComponent, SpotLight2D>(entity);
		//	sceneRenderer->SubmitLight(light, transform.WorldTransform);
		//}

		sceneRenderer->EndScene();
	}

	

	void Scene::OnRenderEditor(Ref<SceneRenderer> sceneRenderer, const Editor::EditorCamera& camera, Timestep ts)
	{
		XYZ_PROFILE_FUNC("Scene::OnRenderEditor");
		m_PhysicsWorld.Step(ts);

		updateHierarchy();
		sceneRenderer->BeginScene(
			camera.GetViewProjection(),
			camera.GetViewMatrix(),
			camera.GetPosition()
		);
		auto renderView = m_ECS.CreateView<TransformComponent, SpriteRenderer>();
		for (auto entity : renderView)
		{
			auto [transform, renderer] = renderView.Get<TransformComponent, SpriteRenderer>(entity);
			if (renderer.Visible && renderer.SubTexture.Raw() && renderer.Material.Raw())
				sceneRenderer->SubmitSprite(
					renderer.Material, renderer.SubTexture, renderer.SortLayer, renderer.Color, transform.WorldTransform
				);
		}
		//auto editorRenderView = m_ECS.CreateView<TransformComponent, EditorSpriteRenderer>();
		//for (auto entity : editorRenderView)
		//{
		//	auto [transform, renderer] = editorRenderView.Get<TransformComponent, EditorSpriteRenderer>(entity);
		//	if (renderer.SubTexture.Raw() && renderer.Material.Raw())
		//		Renderer2D->SubmitSprite(renderer.Material, renderer.SubTexture, renderer.Color, transform.WorldTransform);
		//}
		//auto particleView = m_ECS.CreateView<TransformComponent, ParticleComponentGPU>();
		//for (auto entity : particleView)
		//{
		//	auto [transform, particle] = particleView.Get<TransformComponent, ParticleComponentGPU>(entity);
		//	sceneRenderer->SubmitRendererCommand(particle.System->m_Renderer, transform.WorldTransform);
		//}

		auto particleViewCPU = m_ECS.CreateView<TransformComponent, ParticleComponentCPU, MeshComponent>();
		for (auto entity : particleViewCPU)
		{
			// Render previous frame data
			auto [transform, particle, mesh] = particleViewCPU.Get<TransformComponent, ParticleComponentCPU, MeshComponent>(entity);
			particle.System.SetupForRender(sceneRenderer);
			auto renderData = particle.System.GetRenderDataRead();
			auto data = renderData->m_RenderParticleData.As<ParticleRenderData>();
			mesh.Mesh->SetVertexBufferData(1, data, renderData->m_InstanceCount * renderData->m_RenderParticleData.ElementSize());
			sceneRenderer->SubmitMeshInstanced(mesh.Mesh, transform.WorldTransform, renderData->m_InstanceCount);
		}

		for (auto entity : particleViewCPU)
		{
			// Update ( submit jobs worker threads )
			auto [transform, particle, mesh] = particleViewCPU.Get<TransformComponent, ParticleComponentCPU, MeshComponent>(entity);
			particle.System.SetPhysicsWorld(&m_PhysicsWorld);
			particle.System.Update(ts);
		}
	
		
		//auto lightView = m_ECS.CreateView<TransformComponent, PointLight2D>();
		//for (auto entity : lightView)
		//{
		//	auto [transform, light] = lightView.Get<TransformComponent, PointLight2D>(entity);
		//	sceneRenderer->SubmitLight(light, transform.WorldTransform);
		//}	
		//
		//auto spotLightView = m_ECS.CreateView<TransformComponent, SpotLight2D>();
		//for (auto entity : spotLightView)
		//{
		//	auto [transform, light] = spotLightView.Get<TransformComponent, SpotLight2D>(entity);
		//	sceneRenderer->SubmitLight(light, transform.WorldTransform);
		//}		

		sceneRenderer->EndScene();
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

	SceneEntity Scene::GetSceneEntity()
	{
		return SceneEntity(m_SceneEntity, this);
	}

	SceneEntity Scene::GetSelectedEntity()
	{
		return { m_SelectedEntity, this };
	}

	void Scene::updateHierarchy()
	{
		XYZ_PROFILE_FUNC("Scene::updateHierarchy");
		std::stack<Entity> entities;
		entities.push(m_SceneEntity);
		while (!entities.empty())
		{
			const Entity tmp = entities.top();
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
		ScopedLock<b2World> physicsWorld = m_PhysicsWorld.GetWorld();
		const PhysicsWorld2D::Layer defaultLayer = m_PhysicsWorld.GetLayer(PhysicsWorld2D::DefaultLayer);
		b2FixtureDef fixture;
		fixture.filter.categoryBits = defaultLayer.m_CollisionMask.to_ulong();
		fixture.filter.maskBits = BIT(defaultLayer.m_ID);

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
			
			b2Body* body = physicsWorld->CreateBody(&bodyDef);
			rigidBody.RuntimeBody = body;
			
			if (entity.HasComponent<BoxCollider2DComponent>())
			{
				BoxCollider2DComponent& boxCollider = entity.GetComponent<BoxCollider2DComponent>();
				b2PolygonShape poly;
				
				poly.SetAsBox( boxCollider.Size.x / 2.0f, boxCollider.Size.y / 2.0f, 
					   b2Vec2{ boxCollider.Offset.x, boxCollider.Offset.y }, 0.0f);
				
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
		
				fixture.shape = &circle;
				fixture.density  = circleCollider.Density;
				fixture.friction = circleCollider.Friction;
				circleCollider.RuntimeFixture = body->CreateFixture(&fixture);
			}
			else if (entity.HasComponent<PolygonCollider2DComponent>())
			{
				PolygonCollider2DComponent& meshCollider = entity.GetComponent<PolygonCollider2DComponent>();
				b2PolygonShape poly;
				poly.Set((const b2Vec2*)meshCollider.Vertices.data(), (int32_t)meshCollider.Vertices.size());

				fixture.shape = &poly;
				fixture.density =  meshCollider.Density;
				fixture.friction = meshCollider.Friction;
				meshCollider.RuntimeFixture = body->CreateFixture(&fixture);
			}
			else if (entity.HasComponent<ChainCollider2DComponent>())
			{
				ChainCollider2DComponent& chainCollider = entity.GetComponent<ChainCollider2DComponent>();
				
				b2ChainShape chain;
				chain.CreateChain((const b2Vec2*)chainCollider.Points.data(), (int32_t)chainCollider.Points.size(),
					{ chainCollider.Points[0].x, chainCollider.Points[0].y },
					{ chainCollider.Points.back().x, chainCollider.Points.back().y });
	
				fixture.shape = &chain;
				fixture.density  = chainCollider.Density;
				fixture.friction = chainCollider.Friction;
				chainCollider.RuntimeFixture = body->CreateFixture(&fixture);
			}
			counter++;
		}
	}
}