#pragma once
#include "Window.h"
#include "LayerStack.h"
#include "ThreadPool.h"

#include "XYZ/BasicUI/BasicUILayer.h"
#include "XYZ/InGui/InGuiLayer.h"

namespace XYZ {
	class Application
	{
	public:
		Application();
		virtual ~Application();

		void Run();
		void PushLayer(Layer* layer);
		void PushOverlay(Layer* overlayer);
		void PopLayer(Layer* layer);
		void Stop();


		bool OnEvent(Event& event);

		Window& GetWindow() { return *m_Window; }
		static ThreadPool& GetThreadPool() { return s_ThreadPool; }
		const std::string& GetApplicationDir() const { return m_ApplicationDir; }

		InGuiLayer* GetInGuiLayer() { return m_InGuiLayer; }

		inline static Application& Get() { return *s_Application; }

		static Application* CreateApplication();

	private:
		bool onWindowResized(WindowResizeEvent& event);
		bool onWindowClosed(WindowCloseEvent& event);

	private:
		LayerStack m_LayerStack;
		//bUILayer* m_bUILayer;
		InGuiLayer* m_InGuiLayer;

		std::unique_ptr<Window> m_Window;

		bool m_Running;
		float m_LastFrameTime = 0.0f;

		std::string m_ApplicationDir;

		static ThreadPool s_ThreadPool;
		static Application* s_Application;
	};

}