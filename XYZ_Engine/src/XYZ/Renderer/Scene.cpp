#include "stdafx.h"
#include "Scene.h"
#include "Renderer.h"
#include "Renderer2D.h"
#include "RenderCommand.h"


namespace XYZ {


	Scene::Scene(std::string& name)
		:
		m_Name(name),
		m_Camera(OrthoCamera(1.0f, 1.0f, 1.0f, 1.0f))
	{
		Entity entity = ECSManager::Get().CreateEntity();
		m_World = {
			nullptr,
			ECSManager::Get().AddComponent<Transform2D>(entity,Transform2D{glm::vec3(0,0,0)}),
			entity
		};
		m_Root = m_SceneGraph.InsertNode(Node<SceneObject>(m_World));
		m_SceneGraph.SetRoot(0);
	}

	Scene::~Scene() 
	{
	}

	uint16_t Scene::AddObject(const SceneObject& object)
	{
		uint16_t index = m_SceneGraph.InsertNode(Node<SceneObject>(object));
		m_SceneGraph.SetParent(m_Root, index, SceneSetup());
		return index;
	}

	SceneObject& Scene::GetObject(uint16_t index)
	{
		return m_SceneGraph.GetElement(index);
	}

	void Scene::OnRender()
	{
		RenderCommand::Clear();
		RenderCommand::SetClearColor(glm::vec4(0.2, 0.2, 0.5, 1));

		Renderer2D::BeginScene(m_Camera);

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

	void Scene::OnDetach()
	{
		Renderer2D::Shutdown();

	}

}