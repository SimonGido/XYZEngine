#include "stdafx.h"
#include "Renderer2D.h"

#include "XYZ/ECS/ECSManager.h"

namespace XYZ {
	Renderer2D* Renderer2D::s_Instance = NULL;
	void Renderer2D::Init()
	{
		if (!s_Instance)
		{
			s_Instance = new Renderer2D;
			s_Instance->m_BatchSystem = ECSManager::Get().RegisterSystem<RendererBatchSystem2D>();
		}
	}

	void Renderer2D::Shutdown()
	{
		delete s_Instance;
		s_Instance = NULL;
	}

	void Renderer2D::BeginScene(const OrthoCamera& camera)
	{
		s_Instance->m_BatchSystem->SubmitToRenderer();
	}

	void Renderer2D::Submit(CommandI& command, unsigned int size)
	{
		s_Instance->m_CommandQueue.Allocate(&command, size);
	}

	void Renderer2D::Flush()
	{
		s_Instance->m_CommandQueue.Execute();
	}

	void Renderer2D::EndScene()
	{
	}

}