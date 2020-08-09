#include "stdafx.h"
#include "Scene.h"

#include "XYZ/Core/Input.h"
#include "XYZ/Core/Application.h"
#include "XYZ/Renderer/Renderer.h"

#include "XYZ/Renderer/RenderCommand.h"
#include "XYZ/ECS/Entity.h"

#include "XYZ/Timer.h"

namespace XYZ {


	Scene::Scene(const std::string& name)
		:
		m_Name(name)
	{
		m_ECS = new ECSManager();
		m_ECS->CreateGroup<Transform, SpriteRenderer>();
		m_RenderGroup = m_ECS->GetGroup<Transform, SpriteRenderer>();


		uint32_t entity = m_ECS->CreateEntity();
		m_SceneWorld.Transform = m_ECS->AddComponent(entity, Transform{ glm::vec3(0,0,0) });
		m_SceneWorld.Entity = entity;
			
		
		m_Root = m_SceneGraph.InsertNode(Node<SceneObject>(m_SceneWorld));
		m_SceneGraph.SetRoot(m_Root);
		m_SceneGraph[m_Root].Transform->CalculateWorldTransformation();
		m_SceneGraph[m_Root].Transform->GetTransformation();

		m_MainCameraEntity = m_ECS->CreateEntity();
		m_MainCamera = m_ECS->AddComponent<CameraComponent>(m_MainCameraEntity, CameraComponent{});
		m_MainCameraTransform = m_ECS->AddComponent<Transform>(m_MainCameraEntity, Transform{ {0,0,0} });
		
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
		delete m_ECS;
	}

	Entity Scene::CreateEntity(const std::string& name)
	{
		Entity entity(this);
		entity.AddComponent<SceneTagComponent>({ name });

		SceneObject object;
		object.Transform = entity.AddComponent<Transform>({ glm::vec3(0) });
		object.Entity = entity;
		

		uint16_t id = m_SceneGraph.InsertNode(Node<SceneObject>(object));	
		m_SceneGraph.SetParent(m_Root, id, [](SceneObject& parent, SceneObject& child) {
			child.Transform->SetParent(parent.Transform);
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
		m_SceneGraph.SetParent(parentIndex, childIndex, [](SceneObject& parent, SceneObject& child) {
			child.Transform->SetParent(parent.Transform);
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
			m_ECS->DestroyEntity(it.GetData().Entity);
		}
	}

	void Scene::OnRender()
	{		
		glm::mat4 viewProjMatrix = m_MainCamera->Camera.GetProjectionMatrix() 
			* glm::inverse(m_MainCameraTransform->GetTransformation());

		glm::vec2 winSize = { Input::GetWindowSize().first, Input::GetWindowSize().second };

		
		m_SceneGraph.Propagate([this](SceneObject* parent, SceneObject* child) {
			child->Transform->CalculateWorldTransformation();
		});
		
		// 3D part here

		///////////////

		Renderer2D::BeginScene({ viewProjMatrix ,winSize });
		for (int i = 0; i < m_RenderGroup->Size(); ++i)
		{
			auto [transform, sprite] = (*m_RenderGroup)[i];
			Renderer2D::SetMaterial(sprite->Material);
			Renderer2D::SubmitQuad(transform->GetTransformation(), sprite->SubTexture->GetTexCoords(), sprite->TextureID);
		}
		Renderer2D::Flush();
		Renderer2D::EndScene();
	}

	void Scene::OnRenderEditor(float dt, const SceneRenderData& renderData)
	{	
		m_SceneGraph.Propagate([this](SceneObject* parent, SceneObject* child) {
			child->Transform->CalculateWorldTransformation();
		});

		// 3D part here

		///////////////

	
		Renderer2D::BeginScene(renderData);
		for (int i = 0; i < m_RenderGroup->Size(); ++i)
		{
			auto [transform, sprite] = (*m_RenderGroup)[i];
			Renderer2D::SetMaterial(sprite->Material);
			Renderer2D::SubmitQuad(transform->GetTransformation(), sprite->SubTexture->GetTexCoords(), sprite->TextureID);
		}
		Renderer2D::Flush();
		Renderer2D::EndScene();
			
	}

}