#pragma once
#include "XYZ/Renderer/APIContext.h"
#include "XYZ/Core/Window.h"

#include <GLFW/glfw3.h>


namespace XYZ {
/**	@class WindowsWindow
 *	Enables window management on Windows
 *
 */
	class WindowsWindow : public Window
	{
	public:
		WindowsWindow(const WindowProperties& props);
		virtual ~WindowsWindow();

		virtual void Update() override;
		virtual void SetVSync(bool enabled) override;
		virtual bool IsClosed() override;
		inline virtual void* GetNativeWindow() const { return m_Window; }

		inline virtual uint32_t GetWidth() const override { return m_Data.Width; }
		inline virtual uint32_t GetHeight() const override { return m_Data.Height; }
	private:
		virtual void Destroy();

		GLFWwindow* m_Window;
		std::unique_ptr<APIContext> m_Context;

		struct WindowData
		{
			std::string Title;
			uint32_t Width = 0;
			uint32_t Height = 0;
			bool VSync = false;
		};

		WindowData m_Data;
	};
}