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

		m_ViewportWidth  = 0;
		m_ViewportHeight = 0;

		m_CameraMaterial = Ref<Material>::Create(Shader::Create("Assets/Shaders/DefaultShader.glsl"));
		m_CameraTexture = Texture2D::Create(TextureWrap::Clamp, TextureParam::Linear, TextureParam::Nearest, "Assets/Textures/Gui/Camera.png");
		m_CameraSubTexture = Ref<SubTexture2D>::Create(m_CameraTexture, glm::vec4(0.0f, 0.0f, 1.0f, 1.0f));
		m_CameraMaterial->Set("u_Color", glm::vec4(1.0f));
		m_CameraMaterial->Set("u_Texture", m_CameraTexture);

		m_CameraSprite = new SpriteRenderer(
			m_CameraMaterial,
			m_CameraSubTexture,
			glm::vec4(1.0f),
			0,
			100
		);
	}

	Scene::~Scene() 
	{
		delete m_CameraSprite;
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
			m_MainCamera->Camera.SetViewportSize(m_ViewportWidth, m_ViewportHeight);
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
		// 3D part here

		///////////////
		SceneRendererCamera renderCamera;
		renderCamera.Camera = m_MainCamera->Camera;
		renderCamera.ViewMatrix = glm::inverse(m_MainCameraTransform->GetTransform());
		SceneRenderer::GetOptions().ShowGrid = false;
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

		SceneRendererCamera renderCamera;
		renderCamera.Camera = camera;
		renderCamera.ViewMatrix = camera.GetViewMatrix();
		
		SceneRenderer::GetOptions().ShowGrid = true;
		SceneRenderer::SetGridProperties({ gridTransform,{8.025f * (cameraWidth / camera.GetZoomLevel()), 8.025f * (cameraHeight / camera.GetZoomLevel())},0.025f });
		SceneRenderer::BeginScene(this, renderCamera);
		for (int i = 0; i < m_RenderGroup->Size(); ++i)
		{
			auto [transform, sprite] = (*m_RenderGroup)[i];
			SceneRenderer::SubmitSprite(sprite, transform->GetTransform());
		}
		if (m_SelectedEntity < MAX_ENTITIES)
		{
			if (m_ECS.Contains<CameraComponent>(m_SelectedEntity))
			{
				showCamera(m_SelectedEntity);
			}
			else
			{
				showSelection(m_SelectedEntity);
			}
		}
		SceneRenderer::EndScene();

	}

	void Scene::SetViewportSize(uint32_t width, uint32_t height)
	{
		SceneRenderer::SetViewportSize(width, height);
		m_ViewportWidth = width;
		m_ViewportHeight = height;
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

	void Scene::showCamera(uint32_t entity)
	{
		auto& camera = m_ECS.GetComponent<CameraComponent>(entity)->Camera;
		auto transformComponent = m_ECS.GetComponent<TransformComponent>(entity);

		SceneRenderer::SubmitSprite(m_CameraSprite, transformComponent->GetTransform());

		auto& translation = transformComponent->Translation;
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