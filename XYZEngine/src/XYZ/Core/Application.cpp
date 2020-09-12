#include "stdafx.h"
#include "Application.h"


#include "XYZ/Timer.h"
#include "XYZ/Renderer/Renderer.h"


#include <GLFW/glfw3.h>
#define GLFW_EXPOSE_NATIVE_WIN32
#include <GLFW/glfw3native.h>
#include <Windows.h>


namespace XYZ {
	Application* Application::s_Application = nullptr;


	Application::Application()
	{
		Logger::Get()->SetLogLevel(LogLevel::INFO | LogLevel::WARNING | LogLevel::ERR);
		//Logger::Get()->SetLogLevel(LogLevel::NOLOG);
		s_Application = this;
		m_Running = true;

		m_Window = Window::Create();
		m_Window->SetVSync(false);
		m_Window->SetEventCallback(Hook(&Application::OnEvent, this));

		m_InGuiLayer = new InGuiLayer();
		m_LayerStack.PushOverlay(m_InGuiLayer);
		m_Window->RegisterCallback<WindowCloseEvent>(Hook(&Application::onWindowClosed, this));
		m_Window->RegisterCallback<WindowResizeEvent>(Hook(&Application::onWindowResized, this));
			
	}

	Application::~Application()
	{
		
	}

	void Application::Run()
	{
		while (m_Running)
		{
			float time = (float)glfwGetTime();
			float timestep = time - m_LastFrameTime;
			m_LastFrameTime = time;
			{
				//Stopwatch watch;
				
				for (Layer* layer : m_LayerStack)	
					layer->OnUpdate(timestep);
				
				m_InGuiLayer->Begin();
				for (Layer* layer : m_LayerStack)
					layer->OnInGuiRender(timestep);
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

	void Application::onWindowResized(WindowResizeEvent& event)
	{
		Renderer::OnWindowResize(event.GetWidth(), event.GetHeight());
	}

	void Application::onWindowClosed(WindowCloseEvent& event)
	{
		m_Running = false;
	}


	void Application::OnEvent(Event& event)
	{
		//if (event.GetEventComponent() == EventComponent::WindowResized)
		//{
		//	WindowResizeEvent& resize = (WindowResizeEvent&)event;
		//	Renderer::OnWindowResize(resize.GetWidth(), resize.GetHeight());
		//}
		//else if (event.GetEventComponent() == EventComponent::WindowClosed)
		//{
		//	WindowCloseEvent& close = (WindowCloseEvent&)event;
		//	m_Running = false;
		//}
		for (auto it = m_LayerStack.rbegin(); it != m_LayerStack.rend(); ++it)
		{
			(*it)->OnEvent(event);
			if (event.Handled)
				break;
		}
	}

	std::string Application::OpenFile(const char* filter) const
	{
		OPENFILENAMEA ofn;       // common dialog box structure
		CHAR szFile[260] = { 0 };       // if using TCHAR macros

		// Initialize OPENFILENAME
		ZeroMemory(&ofn, sizeof(OPENFILENAME));
		ofn.lStructSize = sizeof(OPENFILENAME);
		ofn.hwndOwner = glfwGetWin32Window((GLFWwindow*)m_Window->GetNativeWindow());
		ofn.lpstrFile = szFile;
		ofn.nMaxFile = sizeof(szFile);
		ofn.lpstrFilter = filter;
		ofn.nFilterIndex = 1;
		ofn.Flags = OFN_PATHMUSTEXIST | OFN_FILEMUSTEXIST | OFN_NOCHANGEDIR;

		
		if (GetOpenFileNameA(&ofn) == TRUE)
		{
			return ofn.lpstrFile;
		}
		return std::string();

	}

	std::string Application::SaveFile(const char* filter) const
	{
		OPENFILENAMEA ofn;       // common dialog box structure
		CHAR szFile[260] = { 0 };       // if using TCHAR macros

		// Initialize OPENFILENAME
		ZeroMemory(&ofn, sizeof(OPENFILENAME));
		ofn.lStructSize = sizeof(OPENFILENAME);
		ofn.hwndOwner = glfwGetWin32Window((GLFWwindow*)m_Window->GetNativeWindow());
		ofn.lpstrFile = szFile;
		ofn.nMaxFile = sizeof(szFile);
		ofn.lpstrFilter = filter;
		ofn.nFilterIndex = 1;
		ofn.Flags = OFN_PATHMUSTEXIST | OFN_FILEMUSTEXIST | OFN_NOCHANGEDIR;

		if (GetSaveFileNameA(&ofn) == TRUE)
		{
			return ofn.lpstrFile;
		}

	
		return std::string();

	}

	std::string Application::CreateNewFile(const char* filter) const
	{
		HANDLE hFile = INVALID_HANDLE_VALUE;
		OPENFILENAMEA ofn;       // common dialog box structure
		CHAR szFile[260] = { 0 };       // if using TCHAR macros

		// Initialize OPENFILENAME
		ZeroMemory(&ofn, sizeof(OPENFILENAME));
		ofn.lStructSize = sizeof(OPENFILENAME);
		ofn.hwndOwner = glfwGetWin32Window((GLFWwindow*)m_Window->GetNativeWindow());
		ofn.lpstrFile = szFile;
		ofn.nMaxFile = sizeof(szFile);
		ofn.lpstrFilter = filter;
		ofn.nFilterIndex = 1;
		ofn.Flags = OFN_PATHMUSTEXIST | OFN_FILEMUSTEXIST | OFN_NOCHANGEDIR;

		if (GetSaveFileNameA(&ofn) == TRUE)
		{
			hFile = CreateFileA(ofn.lpstrFile, // file name 
				GENERIC_WRITE | GENERIC_READ,        // open for write / read 
				1,                    // do share 
				NULL,                 // default security 
				CREATE_ALWAYS,        // overwrite existing
				FILE_ATTRIBUTE_NORMAL,// normal file 
				NULL
			);                // no template 
		
			CloseHandle(hFile);
			return ofn.lpstrFile;
		}
		CloseHandle(hFile);
		return std::string();
	}
	
}