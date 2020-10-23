#include "stdafx.h"
#include "Scene.h"

#include "XYZ/Core/Input.h"
#include "XYZ/Core/Application.h"
#include "XYZ/Renderer/Renderer.h"

#include "XYZ/NativeScript/ScriptableEntity.h"
#include "XYZ/ECS/Entity.h"

#include "XYZ/Timer.h"

#include <glm/glm.hpp>
#include <glm/gtx/transform.hpp>


namespace XYZ {

	Scene::Scene(const std::string& name)
		:
		m_Name(name)
	{
		m_RenderGroup = m_ECS.CreateGroup<TransformComponent, SpriteRenderer>();
		m_AnimateGroup = m_ECS.CreateGroup<AnimatorComponent>();
		m_ScriptGroup = m_ECS.CreateGroup<NativeScriptComponent>();

	
				
		m_MainCameraEntity = m_ECS.CreateEntity();
		m_MainCamera = m_ECS.EmplaceComponent<CameraComponent>(m_MainCameraEntity);
		m_MainCameraTransform = m_ECS.EmplaceComponent<TransformComponent>(m_MainCameraEntity);
		
		
		m_Entities.push_back(m_MainCameraEntity);
		uint32_t index = m_Entities.size() - 1;
		m_SceneGraphMap.insert({ m_MainCameraEntity, index });

		m_MainCamera->Camera.SetOrthographic(4.0f);	
		m_MainCamera->Camera.SetViewportSize(Application::Get().GetWindow().GetWidth(), Application::Get().GetWindow().GetHeight());	
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
		Renderer::Init();
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
		Renderer2D::BeginScene({ viewProjMatrix });
		for (int i = 0; i < m_RenderGroup->Size(); ++i)
		{
			auto [transform, sprite] = (*m_RenderGroup)[i];
			Renderer2D::SetMaterial(sprite->Material);
			Renderer2D::SubmitQuad(transform->GetTransform(), sprite->SubTexture->GetTexCoords(), sprite->TextureID, sprite->Color);
		}
		Renderer2D::Flush();
		Renderer2D::FlushLines();
		Renderer2D::EndScene();
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

		Renderer2D::BeginScene({camera.GetViewProjectionMatrix()});
		Renderer2D::ShowGrid(gridTransform, glm::vec2(16.025f * camera.GetAspectRatio(), 16.025f ));
		
		for (int i = 0; i < m_RenderGroup->Size(); ++i)
		{
			auto [transform, sprite] = (*m_RenderGroup)[i];
			Renderer2D::SetMaterial(sprite->Material);
			Renderer2D::SubmitQuad(transform->GetTransform(), sprite->SubTexture->GetTexCoords(), sprite->TextureID, sprite->Color);
		}

		
		Renderer2D::Flush();
		Renderer2D::FlushLines();
		Renderer2D::EndScene();		
	}

	Entity Scene::GetEntity(uint32_t index)
	{
		return { m_Entities[index],this };
	}

}