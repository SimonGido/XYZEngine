#include "stdafx.h"
#include "Scene.h"

#include "XYZ/Core/Input.h"
#include "XYZ/Core/Application.h"
#include "XYZ/Renderer/Renderer.h"

#include "XYZ/Renderer/RenderCommand.h"


namespace XYZ {


	Scene::Scene(const std::string& name)
		:
		m_Name(name)
	{
		Entity entity(this);
		
		m_SceneWorld = {
			nullptr,
			entity.AddComponent(Transform2D{glm::vec3(0,0,0)}),
			entity
		};
		
		m_Root = m_SceneGraph.InsertNode(Node<SceneObject>(m_SceneWorld));
		m_SceneGraph.SetRoot(m_Root);
		m_SceneGraph[m_Root].Transform->CalculateWorldTransformation();
		m_SceneGraph[m_Root].Transform->GetTransformation();

		m_MainCamera = m_MainCameraEntity.AddComponent<CameraComponent>(CameraComponent{});
		m_MainCameraTransform = m_MainCameraEntity.AddComponent<Transform2D>(Transform2D{ {0,0,0} });
		
		SceneObject cameraObject = {
			nullptr,
			m_MainCameraTransform,
			m_MainCameraEntity
		};
		
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
		entity.AddComponent<SceneTagComponent>({ name });

		SceneObject object = {
			nullptr,
			entity.AddComponent<Transform2D>({glm::vec3(0)}),
			entity
		};

		uint16_t id = m_SceneGraph.InsertNode(Node<SceneObject>(object));	
		m_SceneGraph.SetParent(m_Root, id, SceneSetup());
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
		m_SceneGraph.SetParent(parentIndex, childIndex, SceneSetup());
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
			it.GetData().Entity.Destroy();
		}
	}

	void Scene::OnRender()
	{	
		glm::mat4 viewProjMatrix = m_MainCamera->Camera.GetProjectionMatrix() 
			* glm::inverse(m_MainCameraTransform->GetTransformation());

		glm::vec2 winSize = { Input::GetWindowSize().first, Input::GetWindowSize().second };

		Renderer2D::BeginScene({ viewProjMatrix ,winSize });
		m_SceneGraph.Propagate([this](SceneObject* parent, SceneObject* child) {

			child->Transform->CalculateWorldTransformation();
			if (child->Renderable)
				m_SortSystem.PushRenderData(child->Renderable, child->Transform);
		});

		m_SortSystem.SubmitToRenderer();
		Renderer2D::EndScene();
	}

	void Scene::OnRenderEditor(float dt, const SceneRenderData& renderData)
	{
		Renderer2D::BeginScene(renderData);
		m_SceneGraph.Propagate([this](SceneObject* parent, SceneObject* child) {

			child->Transform->CalculateWorldTransformation();
			if (child->Renderable)
				m_SortSystem.PushRenderData(child->Renderable, child->Transform);
		});

		m_SortSystem.SubmitToRenderer();
		Renderer2D::EndScene();
	}

}