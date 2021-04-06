#include "stdafx.h"
#include "Application.h"


#include "XYZ/Timer.h"
#include "XYZ/Renderer/Renderer.h"
#include "XYZ/Asset/AssetManager.h"

#include <GLFW/glfw3.h>
#define GLFW_EXPOSE_NATIVE_WIN32
#include <GLFW/glfw3native.h>


namespace XYZ {
	Application* Application::s_Application = nullptr;


	Application::Application()
	{
		Logger::Get().SetLogLevel(LogLevel::INFO | LogLevel::WARNING | LogLevel::ERR | LogLevel::API);
		//Logger::Get().SetLogLevel(LogLevel::NOLOG);
		s_Application = this;
		m_Running = true;

		m_Window = Window::Create();
		m_Window->SetVSync(false);

		m_Window->RegisterCallback(Hook(&Application::OnEvent, this));	

		m_InGuiLayer = new InGuiLayer();
		m_LayerStack.PushOverlay(m_InGuiLayer);

		TCHAR NPath[MAX_PATH];
		GetCurrentDirectory(MAX_PATH, NPath);
		std::wstring tmp(&NPath[0]);
		m_ApplicationDir = std::string(tmp.begin(), tmp.end());

		AssetManager::Init();
		Renderer::Init();
	}

	Application::~Application()
	{
		Renderer::Shutdown();
		AssetManager::Shutdown();
	}

	void Application::Run()
	{
		while (m_Running)
		{
			float time = (float)glfwGetTime();
			float timestep = time - m_LastFrameTime;
			m_LastFrameTime = time;
			{
				Stopwatch watch;
				
				for (Layer* layer : m_LayerStack)	
					layer->OnUpdate(timestep);

				m_InGuiLayer->Begin();
				for (Layer* layer : m_LayerStack)
					layer->OnInGuiRender();
				m_InGuiLayer->End();
			
			}
			m_Window->Update();
		}
	}

	void Application::PushLayer(Layer* layer)
	{
		m_LayerStack.PushLayer(layer);
	}

	void Application::PushOverlay(Layer* overlayer)
	{
		m_LayerStack.PushOverlay(overlayer);		
	}

	void Application::PopLayer(Layer* layer)
	{
		m_LayerStack.PopLayer(layer);
	}

	void Application::Stop()
	{
		m_Running = false;
	}

	bool Application::onWindowResized(WindowResizeEvent& event)
	{
		Renderer::SetViewPort(0, 0, event.GetWidth(), event.GetHeight());
		return false;
	}

	bool Application::onWindowClosed(WindowCloseEvent& event)
	{
		m_Running = false;
		return false;
	}


	bool Application::OnEvent(Event& event)
	{
		EventDispatcher dispatcher(event);
		dispatcher.Dispatch<WindowResizeEvent>(Hook(&Application::onWindowResized, this));
		dispatcher.Dispatch<WindowCloseEvent>(Hook(&Application::onWindowClosed, this));

		for (auto it = m_LayerStack.rbegin(); it != m_LayerStack.rend(); ++it)
		{
			(*it)->OnEvent(event);
			if (event.Handled)
				return true;
		}
		return false;
	}
}