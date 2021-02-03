#include "stdafx.h"
#include "Scene.h"

#include "XYZ/Core/Input.h"
#include "XYZ/Core/Application.h"
#include "XYZ/Renderer/Renderer.h"
#include "XYZ/Renderer/Renderer2D.h"
#include "XYZ/Renderer/SceneRenderer.h"
#include "XYZ/ECS/ComponentGroup.h"
#include "SceneEntity.h"


#include <glm/glm.hpp>
#include <glm/gtx/transform.hpp>

#include "XYZ/Script/ScriptEngine.h"

namespace XYZ {


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

		m_ECS.ForceStorage<ScriptComponent>();
		m_ScriptStorage = m_ECS.GetStorage<ScriptComponent>();
		m_AnimatorStorage = m_ECS.GetStorage<AnimatorComponent>();
	}

	Scene::~Scene()
	{

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


	void Scene::OnAttach()
	{

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
				return;
			}
		}

		for (size_t i = 0; i < m_ScriptStorage->Size(); ++i)
		{

		}

		XYZ_LOG_ERR("No camera found in the scene");
	}

	void Scene::OnEvent(Event& e)
	{
	}

	void Scene::OnDetach()
	{

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
			if (m_RenderView->GetEntity(i) != m_SelectedEntity)
				SceneRenderer::SubmitCollision(&transform, m_RenderView->GetEntity(i));
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
		SceneRenderer::SetViewportSize(width, height);
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