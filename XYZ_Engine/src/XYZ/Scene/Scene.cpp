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
		Entity entity;
		m_World = {
			nullptr,
			entity.AddComponent(Transform2D{glm::vec3(0,0,0)}),
			entity
		};
		m_Root = m_SceneGraph.InsertNode(Node<SceneObject>(m_World));
		m_SceneGraph.SetRoot(0);

		m_MainCamera = m_MainCameraEntity.AddComponent<CameraComponent>(CameraComponent{});
		m_MainCamera->Camera.SetOrthographic(4);
		m_MainCamera->Camera.SetViewportSize(Application::Get().GetWindow().GetWidth(), Application::Get().GetWindow().GetHeight());
	}

	Scene::~Scene() 
	{

	}

	SceneObject& Scene::GetObject(uint16_t index)
	{
		return m_SceneGraph.GetElement(index);
	}

	Entity Scene::CreateEntity(const std::string& name)
	{
		Entity entity;
		entity.AddComponent<SceneTagComponent>({ name });

		SceneObject object = {
			nullptr,
			entity.AddComponent<Transform2D>({glm::vec3(0)}),
			entity
		};
		uint16_t index = m_SceneGraph.InsertNode(Node<SceneObject>(object));
		m_GraphMap.insert({ entity.GetID(),index });

		return entity;
	}

	void Scene::DestroyEntity(Entity entity)
	{
		auto it = m_GraphMap.find(entity.GetID());
		XYZ_ASSERT(it != m_GraphMap.end(), "");
		
		m_SceneGraph.DeleteNode((*it).second);
		m_GraphMap.erase(it);		
	}

	void Scene::OnRender()
	{
		RenderCommand::Clear();
		RenderCommand::SetClearColor(glm::vec4(0.2, 0.2, 0.5, 1));

		//Renderer2D::BeginScene(m_Camera);

		Renderer2D::Flush();
		Renderer2D::EndScene();
	}

	void Scene::SetParent(uint16_t parent, uint16_t child)
	{
		m_SceneGraph.SetParent(parent, child);
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
		m_SceneGraph.RestartIterator();
		while (m_SceneGraph.Next())
		{
			auto it = m_SceneGraph.GetIterator();
			it.GetData().Entity.Destroy();
		}
	}

}