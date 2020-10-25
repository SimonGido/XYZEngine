#include "stdafx.h"
#include "Scene.h"

#include "XYZ/Core/Input.h"
#include "XYZ/Core/Application.h"
#include "XYZ/Renderer/Renderer.h"
#include "XYZ/Renderer/Renderer2D.h"
#include "XYZ/Renderer/SceneRenderer.h"

#include "XYZ/NativeScript/ScriptableEntity.h"
#include "XYZ/ECS/Entity.h"

#include "XYZ/Timer.h"

#include <glm/glm.hpp>
#include <glm/gtx/transform.hpp>


namespace XYZ {

	Scene::Scene(const std::string& name)
		:
		m_Name(name),
		m_SelectedEntity(Entity())
	{
		m_CameraGroup = m_ECS.CreateGroup<TransformComponent, CameraComponent>();
		m_RenderGroup = m_ECS.CreateGroup<TransformComponent, SpriteRenderer>();
		m_AnimateGroup = m_ECS.CreateGroup<AnimatorComponent>();
		m_ScriptGroup = m_ECS.CreateGroup<NativeScriptComponent>();

		m_MainCamera = nullptr;
		m_MainCameraTransform = nullptr;
	}

	Scene::~Scene() 
	{

	}

	Entity Scene::CreateEntity(const std::string& name)
	{
		Entity entity(m_ECS.CreateEntity(), this);
		entity.EmplaceComponent<SceneTagComponent>( name );
		entity.EmplaceComponent<TransformComponent>(glm::vec3(0.0f, 0.0f, 0.0f));
		
		
		m_Entities.push_back(entity);
		uint32_t index = m_Entities.size() - 1;
		m_SceneGraphMap.insert({ entity,index });

		return entity;
	}

	void Scene::DestroyEntity(Entity entity)
	{
		auto it = m_SceneGraphMap.find(entity);
		XYZ_ASSERT(it != m_SceneGraphMap.end(), "");
	
		// Swap with last and delete
		uint32_t lastEntity = m_Entities.back();
		m_Entities[it->second] = std::move(m_Entities.back());
		m_Entities.pop_back();

		m_SceneGraphMap[lastEntity] = it->second;
		m_SceneGraphMap.erase(it);		

		m_ECS.DestroyEntity(entity);
	}

	void Scene::SetParent(Entity parent, Entity child)
	{
	
	}

	void Scene::OnAttach()
	{

	}

	void Scene::OnPlay()
	{
		
		for (int i = 0; i < m_CameraGroup->Size(); ++i)
		{
			auto [transform, camera] = (*m_CameraGroup)[i];
			m_MainCamera = camera;
			m_MainCameraTransform = transform;
			break;
		}
		if (!m_MainCamera)
		{
			XYZ_LOG_ERR("No camera found in the scene");
		}
		else
		{
			m_MainCamera->Camera.SetOrthographic({ 4.0f });
			m_MainCamera->Camera.SetViewportSize(Application::Get().GetWindow().GetWidth(), Application::Get().GetWindow().GetHeight());
		}
	}

	void Scene::OnEvent(Event& e)
	{
	}

	void Scene::OnDetach()
	{
		for (auto entity : m_Entities)
		{
			m_ECS.DestroyEntity(entity);
		}
	}

	void Scene::OnRender()
	{		
		glm::mat4 viewProjMatrix = m_MainCamera->Camera.GetProjectionMatrix() 
			* glm::inverse(m_MainCameraTransform->GetTransform());


		// 3D part here

		///////////////
		SceneRendererCamera renderCamera;
		renderCamera.Camera = m_MainCamera->Camera;
		renderCamera.ViewMatrix = glm::inverse(m_MainCameraTransform->GetTransform());
		SceneRenderer::BeginScene(this, renderCamera);
		for (int i = 0; i < m_RenderGroup->Size(); ++i)
		{
			auto [transform, sprite] = (*m_RenderGroup)[i];
			SceneRenderer::SubmitSprite(sprite, transform->GetTransform());
		}
		SceneRenderer::EndScene();
	}

	void Scene::OnUpdate(Timestep ts)
	{
		for (int i = 0; i < m_AnimateGroup->Size(); ++i)
		{
			auto [animator] = (*m_AnimateGroup)[i];
			animator->Controller->Update(ts);
		}
		for (int i = 0; i < m_ScriptGroup->Size(); ++i)
		{
			auto [script] = (*m_ScriptGroup)[i];
			if (script->ScriptableEntity)
				script->ScriptableEntity->OnUpdate(ts);
		}
	}

	void Scene::OnRenderEditor(const EditorCamera& camera)
	{

		// 3D part here

		///////////////
		float cameraWidth = camera.GetZoomLevel() * camera.GetAspectRatio() * 2;
		float cameraHeight = camera.GetZoomLevel() * 2;
		glm::mat4 gridTransform = glm::translate(glm::mat4(1.0f), camera.GetPosition()) * glm::scale(glm::mat4(1.0f), { cameraWidth,cameraHeight,1.0f });
		Renderer2D::SubmitGrid(gridTransform, glm::vec2(16.025f * camera.GetAspectRatio(), 16.025f), 0.025f);

		SceneRendererCamera renderCamera;
		renderCamera.Camera = camera;
		renderCamera.ViewMatrix = camera.GetViewMatrix();
		SceneRenderer::SetGridProperties({ gridTransform,{8.025f * (cameraWidth / camera.GetZoomLevel()), 8.025f * (cameraHeight / camera.GetZoomLevel())},0.025f });
		SceneRenderer::BeginScene(this, renderCamera);
		for (int i = 0; i < m_RenderGroup->Size(); ++i)
		{
			auto [transform, sprite] = (*m_RenderGroup)[i];
			SceneRenderer::SubmitSprite(sprite, transform->GetTransform());
		}
		if (m_SelectedEntity < MAX_ENTITIES)
		{
			showSelection(m_SelectedEntity);
		}
		SceneRenderer::EndScene();
	}

	void Scene::SetViewportSize(uint32_t width, uint32_t height)
	{
		SceneRenderer::SetViewportSize(width, height);
	}

	Entity Scene::GetEntity(uint32_t index)
	{
		return { m_Entities[index],this };
	}

	void Scene::showSelection(uint32_t entity)
	{
		auto transformComponent = m_ECS.GetComponent<TransformComponent>(entity);
		auto& translation = transformComponent->Translation;
		auto& scale = transformComponent->Scale;

		glm::vec3 topLeft = { translation.x - scale.x / 2,translation.y + scale.y / 2,1 };
		glm::vec3 topRight = { translation.x + scale.x / 2,translation.y + scale.y / 2,1 };
		glm::vec3 bottomLeft = { translation.x - scale.x / 2,translation.y - scale.y / 2,1 };
		glm::vec3 bottomRight = { translation.x + scale.x / 2,translation.y - scale.y / 2,1 };

		Renderer2D::SubmitLine(topLeft, topRight);
		Renderer2D::SubmitLine(topRight, bottomRight);
		Renderer2D::SubmitLine(bottomRight, bottomLeft);
		Renderer2D::SubmitLine(bottomLeft, topLeft);
	}

}