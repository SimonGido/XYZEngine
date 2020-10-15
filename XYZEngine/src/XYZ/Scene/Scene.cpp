#include "stdafx.h"
#include "Scene.h"

#include "XYZ/Core/Input.h"
#include "XYZ/Core/Application.h"
#include "XYZ/Renderer/Renderer.h"

#include "XYZ/NativeScript/ScriptableEntity.h"
#include "XYZ/ECS/Entity.h"

#include "XYZ/Timer.h"


namespace XYZ {

	
	Scene::Scene(const std::string& name)
		:
		m_Name(name)
	{
		m_RenderGroup = m_ECS.CreateGroup<TransformComponent, SpriteRenderer>();
		m_AnimateGroup = m_ECS.CreateGroup<AnimatorComponent>();
		m_ScriptGroup = m_ECS.CreateGroup<NativeScriptComponent>();

		uint32_t entity = m_ECS.CreateEntity();
		m_SceneWorld.Transform = m_ECS.EmplaceComponent<TransformComponent>(entity);
		m_SceneWorld.Entity = entity;
			
		
		m_Root = m_SceneGraph.InsertNode(Node<SceneObject,uint16_t>(m_SceneWorld));
		m_SceneGraph.SetRoot(m_Root);


		m_MainCameraEntity = m_ECS.CreateEntity();
		m_MainCamera = m_ECS.EmplaceComponent<CameraComponent>(m_MainCameraEntity);
		m_MainCameraTransform = m_ECS.EmplaceComponent<TransformComponent>(m_MainCameraEntity);
		
		SceneObject cameraObject;
		cameraObject.Entity = m_MainCameraEntity;
		cameraObject.Transform = m_MainCameraTransform;

		uint16_t cameraIndex = m_SceneGraph.InsertNode(cameraObject);
		m_SceneGraphMap.insert({ m_MainCameraEntity,cameraIndex });

		m_MainCamera->Camera.SetOrthographic(4.0f);
		
		m_MainCamera->Camera.SetViewportSize(Application::Get().GetWindow().GetWidth(), Application::Get().GetWindow().GetHeight());
	
	}

	Scene::~Scene() 
	{

	}

	Entity Scene::CreateEntity(const std::string& name)
	{
		Entity entity(this);
		entity.EmplaceComponent<SceneTagComponent>( name );

		SceneObject object;
		object.Transform = entity.EmplaceComponent<TransformComponent>();
		object.Entity = entity;
		

		uint16_t id = m_SceneGraph.InsertNode(Node<SceneObject, uint16_t>(object));
		m_SceneGraph.SetParent(m_Root, id, [](Node<SceneObject, uint16_t>* parent, Node<SceneObject, uint16_t>* child) {
			//child.Transform->SetParent(parent.Transform);
		});
		m_SceneGraphMap.insert({ entity,id });

		return entity;
	}

	void Scene::DestroyEntity(Entity entity)
	{
		auto it = m_SceneGraphMap.find(entity);
		XYZ_ASSERT(it != m_SceneGraphMap.end(), "");
		
		m_SceneGraph.DeleteNode((*it).second);
		m_SceneGraphMap.erase(it);		

		// In the scene graph last entity is place at the position of deleted one
		uint32_t lastEntity = m_SceneGraph[(*it).second].Entity;
		m_SceneGraphMap[lastEntity] = (*it).second;
	}

	void Scene::SetParent(Entity parent, Entity child)
	{
		auto itParent = m_SceneGraphMap.find(parent);
		XYZ_ASSERT(itParent != m_SceneGraphMap.end(), "Parent entity does not exist");
		auto itChild = m_SceneGraphMap.find(child);
		XYZ_ASSERT(itChild != m_SceneGraphMap.end(), "Child entity does not exist");

		uint16_t parentIndex = itParent->second;
		uint16_t childIndex = itChild->second;
		
		m_SceneGraph[childIndex].Parent = parentIndex;
		m_SceneGraph.SetParent(parentIndex, childIndex, [](Node<SceneObject, uint16_t>* parent, Node<SceneObject, uint16_t>* child) {
			//child.Transform->SetParent(parent.Transform);
		});
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
		for (auto& it : m_SceneGraph.GetFlatData())
		{
			m_ECS.DestroyEntity(it.GetData().Entity);
		}
	}

	void Scene::OnRender()
	{		
		glm::mat4 viewProjMatrix = m_MainCamera->Camera.GetProjectionMatrix() 
			* glm::inverse(m_MainCameraTransform->Transform);

		glm::vec2 winSize = { Input::GetWindowSize().first, Input::GetWindowSize().second };

		
		m_SceneGraph.Propagate([this](Node<SceneObject, uint16_t>* parent, Node<SceneObject, uint16_t>* child) {
			//child->Transform->CalculateWorldTransformation();
		});
		
		// 3D part here

		///////////////
		Renderer2D::BeginScene({ viewProjMatrix ,winSize });
		for (int i = 0; i < m_RenderGroup->Size(); ++i)
		{
			auto [transform, sprite] = (*m_RenderGroup)[i];
			Renderer2D::SetMaterial(sprite->Material);
			Renderer2D::SubmitQuad(*transform, sprite->SubTexture->GetTexCoords(), sprite->TextureID, sprite->Color);
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

	void Scene::OnRenderEditor(const SceneRenderData& renderData)
	{
		m_SceneGraph.Propagate([this](Node<SceneObject, uint16_t>* parent, Node<SceneObject, uint16_t>* child) {
			//child->Transform->CalculateWorldTransformation();
		});

		// 3D part here

		///////////////

		Renderer2D::BeginScene(renderData);
		for (int i = 0; i < m_RenderGroup->Size(); ++i)
		{
			auto [transform, sprite] = (*m_RenderGroup)[i];
			Renderer2D::SetMaterial(sprite->Material);
			Renderer2D::SubmitQuad(*transform, sprite->SubTexture->GetTexCoords(), sprite->TextureID, sprite->Color);
		}
		Renderer2D::Flush();
		Renderer2D::FlushLines();
		Renderer2D::EndScene();		
	}

	Entity Scene::GetEntity(uint16_t index)
	{
		return { m_SceneGraph[index].Entity,this };
	}

}