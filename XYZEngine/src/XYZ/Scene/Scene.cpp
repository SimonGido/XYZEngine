#include "stdafx.h"
#include "Scene.h"
#include "XYZ/ECS/Serialization/ECSSerializer.h"

#include "XYZ/Core/Input.h"
#include "XYZ/Core/Application.h"
#include "XYZ/Renderer/Renderer.h"
#include "XYZ/Renderer/Renderer2D.h"
#include "XYZ/Renderer/SceneRenderer.h"
#include "SceneEntity.h"


#include <glm/glm.hpp>
#include <glm/gtx/transform.hpp>

#include "XYZ/Script/ScriptEngine.h"

namespace XYZ {

	static std::vector<TransformComponent> s_EditTransforms;

	Scene::Scene(const std::string& name)
		:
		m_Name(name),
		m_PhysicsWorld(glm::vec2(0.0f, -9.8f))
	{
		m_ViewportWidth = 0;
		m_ViewportHeight = 0;

		m_CameraTexture = Texture2D::Create({ TextureWrap::Clamp, TextureParam::Linear, TextureParam::Nearest }, "Assets/Textures/Gui/Camera.png");
		m_CameraSubTexture = Ref<SubTexture>::Create(m_CameraTexture, glm::vec4(0.0f, 0.0f, 1.0f, 1.0f));
		m_CameraMaterial = Ref<Material>::Create(Shader::Create("Assets/Shaders/DefaultShader.glsl"));
		m_CameraMaterial->Set("u_Color", glm::vec4(1.0f));
		m_CameraMaterial->Set("u_Texture", m_CameraTexture);
		m_CameraRenderer = SpriteRenderer(m_CameraMaterial, m_CameraSubTexture, glm::vec4(1.0f), 0);

		m_ECS.ForceStorage<ScriptComponent>();
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
		entity.EmplaceComponent<Relationship>();
		entity.EmplaceComponent<SceneTagComponent>(name);
		entity.EmplaceComponent<TransformComponent>(glm::vec3(0.0f, 0.0f, 0.0f));


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
		m_ECS.DestroyEntity(Entity(entity.m_ID));
	}

	void Scene::OnPlay()
	{
		bool foundCamera = false;
		s_EditTransforms.clear();
		s_EditTransforms.resize(m_ECS.GetNumberOfEntities());		
		for (auto entity : m_Entities)
		{
			SceneEntity ent(entity, this);
			s_EditTransforms[entity] = ent.GetComponent<TransformComponent>();
			if (ent.HasComponent<CameraComponent>())
			{
				ent.GetComponent<CameraComponent>().Camera.SetViewportSize(m_ViewportWidth, m_ViewportHeight);
				m_CameraEntity = entity;
				foundCamera = true;
			}
			if (m_ECS.Contains<RigidBody2DComponent>(entity))
			{
				RigidBody2DComponent& rigidBody = m_ECS.GetComponent<RigidBody2DComponent>(entity);
				TransformComponent& transform = m_ECS.GetComponent<TransformComponent>(entity);
				rigidBody.Body = m_PhysicsWorld.CreateBody(glm::vec2(transform.Translation.x, transform.Translation.y), 0.0f);

				if (rigidBody.Type == RigidBody2DComponent::BodyType::Static)
					rigidBody.Body->SetType(PhysicsBody::Type::Static);
				else if (rigidBody.Type == RigidBody2DComponent::BodyType::Dynamic)
					rigidBody.Body->SetType(PhysicsBody::Type::Dynamic);
				else if (rigidBody.Type == RigidBody2DComponent::BodyType::Kinematic)
					rigidBody.Body->SetType(PhysicsBody::Type::Kinematic);

				if (m_ECS.Contains<BoxCollider2DComponent>(entity))
				{
					BoxCollider2DComponent& boxCollider = m_ECS.GetComponent<BoxCollider2DComponent>(entity);
					boxCollider.Shape = m_PhysicsWorld.AddBox2DShape(
						rigidBody.Body,
						-boxCollider.Size / 2.0f,
						boxCollider.Size / 2.0f,
						boxCollider.Density
					);
				}
			}
		}
		if (!foundCamera)
		{
			XYZ_LOG_ERR("No camera found in the scene");
			return;
		}
		auto& scriptStorage = m_ECS.GetStorage<ScriptComponent>();
		for (size_t i = 0; i < scriptStorage.Size(); ++i)
		{
			ScriptComponent& script = scriptStorage.GetComponentAtIndex(i);
			ScriptEngine::OnCreateEntity({ scriptStorage.GetEntityAtIndex(i), this });
		}
	}

	void Scene::OnStop()
	{
		for (auto entity : m_Entities)
		{
			SceneEntity ent(entity, this);
			ent.GetComponent<TransformComponent>() = s_EditTransforms[(uint32_t)entity];
			if (m_ECS.Contains<RigidBody2DComponent>(entity))
			{
				RigidBody2DComponent& rigidBody = m_ECS.GetComponent<RigidBody2DComponent>(entity);
				m_PhysicsWorld.DestroyBody(rigidBody.Body);
				rigidBody.Body = nullptr;
				if (m_ECS.Contains<BoxCollider2DComponent>(entity))
				{
					BoxCollider2DComponent& boxCollider = m_ECS.GetComponent<BoxCollider2DComponent>(entity);
					m_PhysicsWorld.DestroyShape(boxCollider.Shape);
					boxCollider.Shape = nullptr;
				}
			}
		}
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
		renderCamera.ViewMatrix = glm::inverse(cameraTransform.GetTransform());
		
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
			SceneRenderer::SubmitLight(&light, transform.GetTransform());
		}
		
		SceneRenderer::EndScene();
	}

	void Scene::OnUpdate(Timestep ts)
	{
		m_PhysicsWorld.Update(ts);
		
		auto& scriptStorage = m_ECS.GetStorage<ScriptComponent>();
		for (size_t i = 0; i < scriptStorage.Size(); ++i)
		{
			ScriptComponent& scriptComponent = scriptStorage[i];
			ScriptEngine::OnUpdateEntity({ scriptStorage.GetEntityAtIndex(i),this }, ts);
		}
		
		auto& animatorStorage = m_ECS.GetStorage<AnimatorComponent>();
		for (auto & anim : animatorStorage)
		{
			
		}
		
		auto particleView = m_ECS.CreateView<TransformComponent, ParticleComponent>();
		for (auto entity : particleView)
		{
			auto [transform, particle] = particleView.Get<TransformComponent, ParticleComponent>(entity);
			auto material = particle.ComputeMaterial->GetParentMaterial();
			auto materialInstance = particle.ComputeMaterial;
		
			materialInstance->Set("u_Time", ts);
			materialInstance->Set("u_ParticlesInExistence", (int)std::ceil(particle.ParticleEffect->GetEmittedParticles()));
		
			material->GetShader()->Bind();
			materialInstance->Bind();
		
			particle.ParticleEffect->Update(ts);
			material->GetShader()->Compute(32, 32, 1);
		}
		
		auto rigidBodyView = m_ECS.CreateView<TransformComponent, RigidBody2DComponent>();
 		for (auto entity : rigidBodyView)
		{
			auto [transform, rigidBody] = rigidBodyView.Get<TransformComponent, RigidBody2DComponent>(entity);
			transform.Translation.x = rigidBody.Body->GetPosition().x;
			transform.Translation.y = rigidBody.Body->GetPosition().y;
			//transform.Rotation.z = rigidBody.Body->GetAngle();
		}
	}

	void Scene::OnRenderEditor(const EditorCamera& camera)
	{
		SceneRenderer::BeginScene(this, camera.GetViewProjection());
		
		if ((bool)m_SelectedEntity)
		{
			if (m_ECS.Contains<CameraComponent>(m_SelectedEntity))
			{
				SceneRenderer::SubmitSprite(&m_CameraRenderer, &m_ECS.GetComponent<TransformComponent>(m_SelectedEntity));
				showCamera(m_SelectedEntity);
			}
			else
				showSelection(m_SelectedEntity);
		}

		auto renderView = m_ECS.CreateView<TransformComponent, SpriteRenderer>();
		for (auto entity : renderView)
		{
			auto [transform, renderer] = renderView.Get<TransformComponent, SpriteRenderer>(entity);
			if (renderer.IsVisible && renderer.SubTexture.Raw())
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
	SceneEntity Scene::GetEntityByName(const std::string& name)
	{
		return { m_ECS.FindEntity<SceneTagComponent>(name), this };
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