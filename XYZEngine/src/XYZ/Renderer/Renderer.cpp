#include "stdafx.h"
#include "Renderer.h"
#include "RenderCommand.h"
#include "Renderer2D.h"
#include "InGuiRenderer.h"

namespace XYZ {
	Renderer* Renderer::s_Instance = nullptr;
	Renderer::SceneData* Renderer::s_SceneData = nullptr;

	void Renderer::Init()
	{
		s_Instance = new Renderer;
		s_SceneData = new SceneData;
		RenderCommand::Init();
		Renderer2D::Init();
		InGuiRenderer::Init();
	}

	void Renderer::Shutdown()
	{
		Renderer2D::Shutdown();
		InGuiRenderer::Shutdown();
		delete s_Instance;
		delete s_SceneData;
	}

	void Renderer::OnWindowResize(uint32_t width, uint32_t height)
	{
		RenderCommand::SetViewPort(0, 0, width, height);
	}

	void Renderer::BeginScene(const SceneData& sceneData)
	{
		*s_SceneData = sceneData;
	}
	void Renderer::EndScene()
	{

	}
	void Renderer::Submit(CommandI& command, unsigned int size)
	{
		s_Instance->m_CommandQueue.Allocate(&command, size);
	}
	void Renderer::Flush()
	{
		s_Instance->m_CommandQueue.Execute();
	}
}