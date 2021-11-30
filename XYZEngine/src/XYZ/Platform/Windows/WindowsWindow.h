#pragma once
#include "XYZ/Core/Window.h"
#include "XYZ/Event/EventHandler.h"
#include "XYZ/Event/Event.h"
#include "XYZ/Event/EventSystem.h"

#include <GLFW/glfw3.h>



namespace XYZ {

	class WindowsWindow : public Window
	{
	public:
		WindowsWindow(const WindowProperties& props, const Ref<APIContext>& context);
		virtual ~WindowsWindow() override;

		virtual void BeginFrame() override;
		virtual void SwapBuffers() override;
		virtual void ProcessEvents() override;
		
		virtual void  SetVSync(bool enable) override;
		virtual void  SetStandardCursor(uint8_t cursor) override;
		virtual void  SetCustomCursor(void* cursor) override;
		virtual void* CreateCustomCursor(uint8_t* pixels, uint32_t width, uint32_t height, int32_t xOffset = 0, int32_t yOffset = 0) override;

		virtual bool  IsClosed() const override;
		virtual bool  IsVSync() const override;
		virtual void* GetWindow() const override;
		virtual void* GetNativeWindow() const override;

		inline virtual uint32_t GetWidth() const override { return m_Data.Width; }
		inline virtual uint32_t GetHeight() const override { return m_Data.Height; }
	
	private:
		void destroy() const;

		GLFWwindow* m_Window;

		uint8_t m_CurrentCursor = XYZ_ARROW_CURSOR;
		GLFWcursor* m_Cursors[NUM_CURSORS];

		Ref<APIContext> m_Context;
		
		struct WindowData
		{
			std::string Title;
			uint32_t Width = 0;
			uint32_t Height = 0;
			bool VSync = false;
			WindowsWindow* This = nullptr;
		};

		WindowData m_Data;
		
	};
}