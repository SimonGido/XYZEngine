#include "stdafx.h"
#include "Application.h"

#include "XYZ/Debug/Profiler.h"

#include "XYZ/Renderer/Renderer.h"
#include "XYZ/Asset/AssetManager.h"
#include "XYZ/Audio/Audio.h"


#include "XYZ/API/Vulkan/VulkanContext.h"
#include "XYZ/API/Vulkan/VulkanAllocator.h"
#include "XYZ/API/Vulkan/VulkanRendererAPI.h"

#include "XYZ/ImGui/ImGui.h"


#include <GLFW/glfw3.h>
#define GLFW_EXPOSE_NATIVE_WIN32
#include <GLFW/glfw3native.h>

#include <imgui.h>
#include <backends/imgui_impl_glfw.h>

namespace XYZ {
	Application* Application::s_Application = nullptr;


	Application::Application(const ApplicationSpecification& specification)
		:
		m_LastFrameTime(0.0f),
		m_Timestep(0.0f),
		
		m_Minimized(false),
		m_Specification(specification)
	{	
		m_ThreadPool.Start(std::thread::hardware_concurrency() - 1);
		s_Application = this;
		m_Running = true;
		m_ImGuiLayer = nullptr;

		AssetManager::Init();
		if (specification.WindowCreate)
		{		
			Renderer::Init();
			m_Window = Window::Create(Renderer::GetAPIContext());
			m_Window->RegisterCallback(Hook(&Application::OnEvent, this));
			m_Window->SetVSync(false);
				
			if (specification.EnableImGui)
			{			
				m_ImGuiLayer = ImGuiLayer::Create();
				m_LayerStack.PushOverlay(m_ImGuiLayer);
			}
		}
				
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
		m_ThreadPool.Stop(); // Thread pool must be stopped before renderer, so resources are destroyed properly

		if (m_Specification.WindowCreate)
		{
			Renderer::Shutdown();
		}
	}

	void Application::Run()
	{
		m_Timer.Restart();
		if (m_Specification.WindowCreate)
		{
			onRunWindow();
		}
		else
		{
			onRunWindowless();
		}
		onStop();
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
		const float time = m_Timer.Elapsed() * 0.001f;
		
		m_Timestep = time - m_LastFrameTime;
		m_LastFrameTime = time;
	}

	void Application::onImGuiRender()
	{
		XYZ_PROFILE_FUNC("Application::onImGuiRender");
		XYZ_SCOPE_PERF("Application::onImGuiRender");
		if (m_ImGuiLayer)
		{
			m_ImGuiLayer->Begin();
			displayPerformance();
			displayRenderer();
			for (Layer* layer : m_LayerStack)
				layer->OnImGuiRender();

			m_ImGuiLayer->End();
		}
	}

	void Application::displayPerformance()
	{
		if (ImGui::Begin("Performance"))
		{
			if (ImGui::BeginTable("##PerformanceTable", 2, ImGuiTableFlags_SizingFixedFit))
			{
				UI::TextTableRow("%s", "Frame Time:", "%.2f ms", m_Timestep.GetMilliseconds());
				auto scopeLockedData = m_Profiler.GetPerformanceData();
				for (const auto [name, time] : scopeLockedData.As())
				{
					UI::TextTableRow("%s:", name, "%.3f ms", time);
				}
				ImGui::EndTable();
			}
		}
		ImGui::End();
	}

	void Application::displayRenderer()
	{
		if (ImGui::Begin("Renderer"))
		{
			auto& caps = Renderer::GetCapabilities();
			if (ImGui::BeginTable("##RendererTable", 2, ImGuiTableFlags_SizingFixedFit))
			{
				UI::TextTableRow("%s", "Vendor:", "%s", caps.Vendor.c_str());
				UI::TextTableRow("%s", "Renderer:", "%s", caps.Device.c_str());
				UI::TextTableRow("%s", "Version:", "%s", caps.Version.c_str());

				ImGui::EndTable();
			}
			ImGui::Separator();
			if (RendererAPI::GetType() == RendererAPI::Type::Vulkan)
			{
				if (ImGui::BeginTable("##VulkanAllocatorTable", 2, ImGuiTableFlags_SizingFixedFit))
				{
					GPUMemoryStats memoryStats = VulkanAllocator::GetStats();
					std::string used = Utils::BytesToString(memoryStats.Used);
					std::string free = Utils::BytesToString(memoryStats.Free);

					UI::TextTableRow("%s", "Used VRAM:", "%s", used.c_str());
					UI::TextTableRow("%s", "Free VRAM:", "%s", free.c_str());
					
					ImGui::EndTable();
				}
			}
			bool vSync = m_Window->IsVSync();
			if (ImGui::Checkbox("Vsync", &vSync))
				m_Window->SetVSync(vSync);
		}
		ImGui::End();
	}

	void Application::onStop()
	{
		// Make sure we actually finished all commands submitted last frame
		Renderer::BlockRenderThread(); // Sync before new frame				
		Renderer::Render();
		XYZ_PROFILER_SHUTDOWN;
	}

	void Application::onRunWindow()
	{
		while (m_Running)
		{
			XYZ_PROFILE_FRAME("MainThread");
			updateTimestep();
			m_Window->ProcessEvents();
			if (!m_Minimized)
			{
				Renderer::BlockRenderThread(); // Sync before new frame				
				Renderer::Render();

				m_Window->BeginFrame();
				Renderer::BeginFrame();
				{
					XYZ_SCOPE_PERF("Application Layer::OnUpdate");
					for (Layer* layer : m_LayerStack)
						layer->OnUpdate(m_Timestep);
				}
				if (m_Specification.EnableImGui)
				{
					// We must block render thread only if multiple viewports are created
					if (m_ImGuiLayer->IsMultiViewport())
						Renderer::BlockRenderThread(); // Prevents calling VkSubmitQueue from multiple threads at once
					onImGuiRender();
				}
				Renderer::EndFrame();
				m_Window->SwapBuffers();
			}
			AssetManager::Update(m_Timestep);
		}
	}

	void Application::onRunWindowless()
	{
		while (m_Running)
		{
			XYZ_PROFILE_FRAME("MainThread");
			updateTimestep();
			
			{
				XYZ_SCOPE_PERF("Application Layer::OnUpdate");
				for (Layer* layer : m_LayerStack)
					layer->OnUpdate(m_Timestep);
			}
				
			AssetManager::Update(m_Timestep);
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