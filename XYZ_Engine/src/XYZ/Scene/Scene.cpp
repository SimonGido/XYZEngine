#include "stdafx.h"
#include "Scene.h"

#include "XYZ/Core/Application.h"
#include "XYZ/Renderer/Renderer.h"
#include "XYZ/Renderer/Renderer2D.h"
#include "XYZ/Renderer/RenderCommand.h"


namespace XYZ {


	Scene::Scene(const std::string& name)
		:
		m_Name(name)
	{
		Entity entity(this);
		m_World = {
			nullptr,
			entity.AddComponent(Transform2D{glm::vec3(0,0,0)}),
			entity
		};
		
		m_Root = m_SceneGraph.InsertNode(Node<SceneObject>(m_World));
		m_SceneGraph.SetRoot(0);
		m_SceneGraph[0].Transform->CalculateWorldTransformation();
		m_SceneGraph[0].Transform->GetTransformation();

		m_MainCamera = m_MainCameraEntity.AddComponent<CameraComponent>(CameraComponent{});
		m_MainCameraTransform = m_MainCameraEntity.AddComponent<Transform2D>(Transform2D{ {0,0,0} });
		
		SceneObject cameraObject = {
			nullptr,
			m_MainCameraTransform,
			m_MainCameraEntity
		};
		
		uint16_t id = m_SceneGraph.InsertNode(cameraObject);
		m_GraphMap.insert({ m_MainCameraEntity,id });

		m_MainCamera->Camera.SetOrthographic(4);
		m_MainCamera->Camera.SetViewportSize(Application::Get().GetWindow().GetWidth(), Application::Get().GetWindow().GetHeight());
	}

	Scene::~Scene() 
	{

	}

	SceneObject& Scene::GetObject(uint16_t index)
	{
		return m_SceneGraph[index];
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
		uint16_t index = m_SceneGraph.InsertNode(Node<SceneObject>(object));
		m_SceneGraph.SetParent(0, index, SceneSetup());

		m_GraphMap.insert({ entity,index });

		return entity;
	}

	void Scene::DestroyEntity(Entity entity)
	{
		auto it = m_GraphMap.find(entity);
		XYZ_ASSERT(it != m_GraphMap.end(), "");
		
		m_SceneGraph.DeleteNode((*it).second);
		m_GraphMap.erase(it);		
	}

	void Scene::SetParent(uint16_t parent, uint16_t child)
	{
		m_SceneGraph.SetParent(parent, child, SceneSetup());
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
		glm::mat4 viewProjectionMatrix = m_MainCamera->Camera.GetProjectionMatrix() 
			* glm::inverse(m_MainCameraTransform->GetTransformation());

		Renderer2D::BeginScene(viewProjectionMatrix);
		m_SceneGraph.Propagate([this](SceneObject* parent, SceneObject* child) {

			child->Transform->CalculateWorldTransformation();
			if (child->Renderable)
				m_SortSystem.PushRenderData(child->Renderable, child->Transform);
		});

		m_SortSystem.SubmitToRenderer();
		Renderer2D::EndScene();
	}

	void Scene::OnRenderEditor(float dt, const glm::mat4& viewProjectionMatrix)
	{
		Renderer2D::BeginScene(viewProjectionMatrix);
		m_SceneGraph.Propagate([this](SceneObject* parent, SceneObject* child) {

			child->Transform->CalculateWorldTransformation();
			if (child->Renderable)
				m_SortSystem.PushRenderData(child->Renderable, child->Transform);
		});

		m_SortSystem.SubmitToRenderer();
		Renderer2D::EndScene();
	}

}