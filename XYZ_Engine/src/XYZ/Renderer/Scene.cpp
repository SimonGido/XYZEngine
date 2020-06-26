#include "stdafx.h"
#include "Scene.h"
#include "Renderer.h"
#include "Renderer2D.h"
#include "RenderCommand.h"

XYZ::Scene::Scene(std::string& name)
	: 
	m_Name(name),
	m_Camera(OrthoCamera(1.0f, 1.0f, 1.0f, 1.0f))
{
	
}

XYZ::Scene::~Scene() {}

void XYZ::Scene::AddEntity(Entity entity)
{	
	
}

void XYZ::Scene::OnRender()
{
	RenderCommand::Clear();
	RenderCommand::SetClearColor(glm::vec4(0.2, 0.2, 0.5, 1));

	Renderer2D::BeginScene(m_Camera);

	Renderer2D::Flush();
	Renderer2D::EndScene();
}

void XYZ::Scene::OnAttach()
{
	Renderer::Init();
}

void XYZ::Scene::OnDetach()
{
	Renderer2D::Shutdown();

}