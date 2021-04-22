#pragma once
#include "Window.h"
#include "LayerStack.h"
#include "XYZ/BasicUI/BasicUILayer.h"

/**	@class Application
 *  Singleton class.
 *	Represents main application, stores and updates the Layers and the Window.
 *
 */

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


		bool OnEvent(Event& event);

		Window& GetWindow() { return *m_Window; }
		const std::string& GetApplicationDir() const { return m_ApplicationDir; }

		void Stop();
		inline static Application& Get() { return *s_Application; }

		static Application* CreateApplication();

	private:
		bool onWindowResized(WindowResizeEvent& event);
		bool onWindowClosed(WindowCloseEvent& event);

	private:
		LayerStack m_LayerStack;
		bUILayer* m_bUILayer;

		std::unique_ptr<Window> m_Window;

		bool m_Running;
		float m_LastFrameTime = 0.0f;

		std::string m_ApplicationDir;

		static Application* s_Application;
	};

}