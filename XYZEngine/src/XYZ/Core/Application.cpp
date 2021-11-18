#include "stdafx.h"
#include "Application.h"


#include "XYZ/Debug/Timer.h"
#include "XYZ/Debug/Profiler.h"

#include "XYZ/Renderer/Renderer.h"
#include "XYZ/Asset/AssetManager.h"
#include "XYZ/Audio/Audio.h"
#include "XYZ/API/Vulkan/VulkanContext.h"

#include <GLFW/glfw3.h>
#define GLFW_EXPOSE_NATIVE_WIN32
#include <GLFW/glfw3native.h>

#include <imgui.h>

namespace XYZ {
	Application* Application::s_Application = nullptr;


	Application::Application(const ApplicationSpecification& specification)
		:
		m_LastFrameTime(0.0f),
		m_Timestep(0.0f),
		m_ThreadPool(12),
		m_Minimized(false),
		m_Specification(specification)
	{	
		s_Application = this;
		m_Running = true;

		Renderer::Init();
		m_Window = Window::Create(Renderer::GetAPIContext());
		m_Window->RegisterCallback(Hook(&Application::OnEvent, this));	
		m_Window->SetVSync(0);
		
		m_ImGuiLayer = nullptr;
		m_ImGuiLayer = ImGuiLayer::Create();
		m_LayerStack.PushOverlay(m_ImGuiLayer);

		Renderer::InitResources();
		//AssetManager::Init();

		TCHAR NPath[MAX_PATH];
		GetCurrentDirectory(MAX_PATH, NPath);
		std::wstring tmp(&NPath[0]);
		m_ApplicationDir = std::string(tmp.begin(), tmp.end());	
	}

	Application::~Application()
	{
		for (const auto layer : m_LayerStack) // Make sure that layers are deleted before window
		{
			layer->OnDetach();
			delete layer;
		}
		m_LayerStack.Clear();

		AssetManager::Shutdown();
		Audio::ShutDown();
		Renderer::Shutdown();
	}

	void Application::Run()
	{
		while (m_Running)
		{				
			updateTimestep();
			XYZ_PROFILE_FRAME("MainThread");
			
			m_Window->ProcessEvents();
			if (!m_Minimized)
			{							
				for (Layer* layer : m_LayerStack)
					layer->OnUpdate(m_Timestep);
						
			
				m_Window->BeginFrame();
				if (m_Specification.EnableImGui)
					onImGuiRender();
				Renderer::WaitAndRender();
				m_Window->SwapBuffers();
			}
		}
		XYZ_PROFILER_SHUTDOWN;
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
		const uint32_t width = event.GetWidth(), height = event.GetHeight();	
		if (width == 0 || height == 0)
		{
			m_Minimized = true;
			return false;
		}
		m_Minimized = false;
		Renderer::SetViewPort(0, 0, event.GetWidth(), event.GetHeight());
		Renderer::GetAPIContext()->OnResize(event.GetWidth(), event.GetHeight());	
		return false;
	}

	bool Application::onWindowClosed(WindowCloseEvent& event)
	{
		m_Running = false;
		return false;
	}

	void Application::updateTimestep()
	{
		const float time = static_cast<float>(glfwGetTime());
		m_Timestep = time - m_LastFrameTime;
		m_LastFrameTime = time;
	}

	void Application::onImGuiRender()
	{
		if (m_ImGuiLayer)
		{
			m_ImGuiLayer->Begin();
			if (ImGui::Begin("Stats"))
			{
				ImGui::Text("Performance: %.2f ms", m_Timestep.GetMilliseconds());
			}
			ImGui::End();

			for (Layer* layer : m_LayerStack)
				layer->OnImGuiRender();

			m_ImGuiLayer->End();
		}
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