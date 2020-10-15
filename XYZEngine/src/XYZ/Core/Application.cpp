#include "stdafx.h"
#include "Application.h"


#include "XYZ/Timer.h"
#include "XYZ/Renderer/Renderer.h"


#include <GLFW/glfw3.h>
#define GLFW_EXPOSE_NATIVE_WIN32
#include <GLFW/glfw3native.h>
#include <Windows.h>
#include <shlobj.h>

#include <filesystem>

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
				Stopwatch watch;
				
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
		Renderer::SetViewPort(0, 0, event.GetWidth(), event.GetHeight());
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
		CHAR szFile[MAX_PATH] = { 0 };       // if using TCHAR macros
		
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
		CHAR szFile[MAX_PATH] = { 0 };       // if using TCHAR macros

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
		CHAR szFile[MAX_PATH] = { 0 };       // if using TCHAR macros

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

	static LPITEMIDLIST ConvertPathToLpItemIdList(const char* pszPath)
	{
		LPITEMIDLIST  pidl = NULL;
		LPSHELLFOLDER pDesktopFolder = NULL;
		OLECHAR       olePath[MAX_PATH];
		HRESULT       hr;

		if (SUCCEEDED(SHGetDesktopFolder(&pDesktopFolder)))
		{
			MultiByteToWideChar(CP_ACP, MB_PRECOMPOSED, pszPath, -1, olePath, MAX_PATH);		
			hr = pDesktopFolder->ParseDisplayName(NULL, NULL, olePath, NULL, &pidl, NULL);
			pDesktopFolder->Release();
			return pidl;
		}
		return NULL;
	}

	std::string Application::OpenFolder() const
	{
		std::string path(std::filesystem::current_path().u8string());
		BROWSEINFOA bi;
		bi.hwndOwner = glfwGetWin32Window((GLFWwindow*)m_Window->GetNativeWindow());
		bi.pidlRoot = ConvertPathToLpItemIdList(path.c_str());
		bi.pszDisplayName = NULL;
		bi.lpszTitle = NULL;
		bi.ulFlags = BIF_DONTGOBELOWDOMAIN | BIF_RETURNONLYFSDIRS | BIF_STATUSTEXT | BIF_USENEWUI;
		bi.lpfn = NULL;
		bi.iImage = 0;

		
		LPITEMIDLIST pidl = SHBrowseForFolderA(&bi);
		if (pidl == NULL)
		{
			return std::string();
		}
	
		CHAR strFolder[MAX_PATH];
		SHGetPathFromIDListA(pidl, strFolder);
		
		return strFolder;
	}
	
}