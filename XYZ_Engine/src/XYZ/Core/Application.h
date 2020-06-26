#pragma once
#include "Window.h"
#include "LayerStack.h"
#include "Event/EventManager.h"
#include "XYZ/ImGui/ImGuiLayer.h"
#include <vector>


/**	@class Application
 *  Singleton class.
 *	Represents main application, stores and updates the Layers and the Window.
 *
 */

namespace XYZ {
	class Application
	{
	public:
		/**
		* Constructor
		*/
		Application();

		/**
		* Destructor
		* virtual destructor
		*/
		virtual ~Application();

		/**
		* Main loop of application
		*/
		void Run();

		/**
		* Push layer to the vector of layers, calls OnAttach function for the pushed layer
		* @param layer pointer to the Layer
		*/
		void PushLayer(Layer* layer);

		/**
		* Push layer to the back of the vector of layers, calls OnAttach function for the 
		* pushed layer
		* @param layer pointer to the Layer
		*/
		void PushOverlay(Layer* overlayer);

		/**
		* Finds layer in linear time, calls OnDetach function for the layer and erases it from
		* the container
		* @param layer pointer to the Layer
		*/
		void PopLayer(Layer* layer);

		/**
		* Return handler of the main Window
		* @param[out] reference to the Window
		*/
		Window& GetWindow() { return *m_Window; }

		void Stop();
		/**
		* Return handler of the Application
		* @param[out] reference to the Application
		*/
		inline static Application& Get() { return *s_Application; }

		/**
		* Create the application and returns the pointer
		* @param[out] pointer to the Application
		*/
		static Application* CreateApplication();
	private:
		/**
		* Handler function for the Window Close Event 
		* @param event shared_ptr of type Event
		*/
		void OnWindowClose(event_ptr event);

		/**
		* Handler function for the Window Resize Event
		* @param event shared_ptr of type Event
		*/
		void OnWindowResize(event_ptr event);
	private:
		HandlerID m_WindowResize;
		HandlerID m_WindowClose;

		LayerStack m_LayerStack;
		std::unique_ptr<Window> m_Window;
		ImGuiLayer * m_ImGuiLayer;

		bool m_Running;
		float m_LastFrameTime = 0.0f;

		static Application* s_Application;
	};

}