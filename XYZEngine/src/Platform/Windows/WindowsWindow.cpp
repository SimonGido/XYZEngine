#include "stdafx.h"
#include "WindowsWindow.h"

#include "XYZ/Renderer/Renderer.h"
#include "XYZ/Core/Application.h"

#include <GL/glew.h>

#define GLFW_EXPOSE_NATIVE_WIN32
#include <GLFW/glfw3native.h>

namespace XYZ {
	static bool GLFWInitialized = false;

	std::unique_ptr<Window> Window::Create(const WindowProperties& props)
	{
		return std::make_unique<WindowsWindow>(props);
	}
	

	WindowsWindow::WindowsWindow(const WindowProperties& props)
	{
		if (!GLFWInitialized)
		{
			int success = glfwInit();

			XYZ_ASSERT(success, "Could not initialize GLFW!");
			GLFWInitialized = true;
		}

		m_Data.This = this;
		GLFWmonitor* primaryMonitor = glfwGetPrimaryMonitor();
		const GLFWvidmode* mode = glfwGetVideoMode(primaryMonitor);


		m_Data.Width = props.Width;
		m_Data.Height = props.Height;
		
		if (props.Flags & WindowFlags::MAXIMIZED)
		{
			glfwWindowHint(GLFW_MAXIMIZED, true);
			m_Window = glfwCreateWindow(mode->width, mode->width, props.Title.c_str(), NULL, NULL);	
			m_Data.Width = mode->width;
			m_Data.Height = mode->height;
		}
		else if (props.Flags & WindowFlags::FULLSCREEN)
		{
			m_Window = glfwCreateWindow(mode->width, mode->width, props.Title.c_str(), glfwGetPrimaryMonitor(), NULL);
			m_Data.Width = mode->width;
			m_Data.Height = mode->height;
		}
		else
		{
			m_Window = glfwCreateWindow(props.Width, props.Height, props.Title.c_str(), NULL, NULL);
		}

		m_Context = APIContext::Create(m_Window);	
		#ifdef RENDER_THREAD_ENABLED
		auto result = Renderer::GetPool().PushJob<bool>([this]() ->bool {
			m_Context->Init();
			return true;
		});
		result.wait();
		#else
		m_Context->Init();
		#endif
		glfwSetWindowUserPointer(m_Window, &m_Data);

		
		glfwSetWindowSizeCallback(m_Window, [](GLFWwindow* window, int width, int height)
		{
			WindowData& data = *(WindowData*)glfwGetWindowUserPointer(window);
			data.Width = width;
			data.Height = height;
			WindowResizeEvent e(width, height);
			data.This->Execute(e);
		});

		glfwSetWindowCloseCallback(m_Window, [](GLFWwindow* window)
		{
			WindowData& data = *(WindowData*)glfwGetWindowUserPointer(window);
			WindowCloseEvent e;
			data.This->Execute(e);
		});

		glfwSetCharCallback(m_Window, [](GLFWwindow* window, unsigned int key)
		{
			WindowData& data = *(WindowData*)glfwGetWindowUserPointer(window);
			KeyTypedEvent e(key);
			data.This->Execute(e);
		});

		glfwSetKeyCallback(m_Window, [](GLFWwindow* window, int key, int scancode, int action, int mods)
		{
			WindowData& data = *(WindowData*)glfwGetWindowUserPointer(window);

			switch (action)
			{
			case GLFW_PRESS:
			{
				KeyPressedEvent e(key, mods);
				data.This->Execute(e);
				break;
			}
			case GLFW_RELEASE:
			{
				KeyReleasedEvent e(key);
				data.This->Execute(e);
				break;
			}

			}
		});

		glfwSetMouseButtonCallback(m_Window, [](GLFWwindow* window, int button, int action, int mods)
		{
			WindowData& data = *(WindowData*)glfwGetWindowUserPointer(window);
			switch (action)
			{
			case GLFW_PRESS:
			{
				MouseButtonPressEvent e(button);
				data.This->Execute(e);
				break;
			}
			case GLFW_RELEASE:
			{
				MouseButtonReleaseEvent e(button);
				data.This->Execute(e);
				break;
			}

			}
		});

		glfwSetScrollCallback(m_Window, [](GLFWwindow* window, double xOffset, double yOffset)
		{
			WindowData& data = *(WindowData*)glfwGetWindowUserPointer(window);
			MouseScrollEvent e((float)xOffset, (float)yOffset);
			data.This->Execute(e);
		});

		glfwSetCursorPosCallback(m_Window, [](GLFWwindow* window, double xPos, double yPos)
		{
			WindowData& data = *(WindowData*)glfwGetWindowUserPointer(window);
			MouseMovedEvent e((int)xPos, (int)yPos);
			data.This->Execute(e);
		});


		m_Cursors[XYZ_ARROW_CURSOR] = glfwCreateStandardCursor(GLFW_ARROW_CURSOR);
		m_Cursors[XYZ_IBEAM_CURSOR] = glfwCreateStandardCursor(GLFW_IBEAM_CURSOR);
		m_Cursors[XYZ_CROSSHAIR_CURSOR] = glfwCreateStandardCursor(GLFW_CROSSHAIR_CURSOR);
		m_Cursors[XYZ_HAND_CURSOR] = glfwCreateStandardCursor(GLFW_HAND_CURSOR);
		m_Cursors[XYZ_HRESIZE_CURSOR] = glfwCreateStandardCursor(GLFW_HRESIZE_CURSOR);
		m_Cursors[XYZ_VRESIZE_CURSOR] = glfwCreateStandardCursor(GLFW_VRESIZE_CURSOR);

		{
			int width, height;
			glfwGetWindowSize(m_Window, &width, &height);
			m_Data.Width = width;
			m_Data.Height = height;
		}
	}

	WindowsWindow::~WindowsWindow()
	{
	}

	void WindowsWindow::Update()
	{
		Renderer::BlockRenderThread();
		glfwPollEvents();				
		m_Context->SwapBuffers();
	}

	void WindowsWindow::SetVSync(int32_t frames)
	{
		#ifdef RENDER_THREAD_ENABLED
		Renderer::GetPool().PushJob<void>([frames]() {
			glfwSwapInterval(frames);
		});
		#else
		glfwSwapInterval(frames);
		#endif
	}

	bool WindowsWindow::IsClosed()
	{
		return glfwWindowShouldClose(m_Window);
	}

	void WindowsWindow::SetStandardCursor(uint8_t cursor)
	{
		XYZ_ASSERT(cursor < NUM_CURSORS, "Invalid cursor");
		if (cursor != m_CurrentCursor)
		{
			m_CurrentCursor = cursor;
			glfwSetCursor(m_Window, m_Cursors[cursor]);
		}
	}

	void WindowsWindow::SetCustomCursor(void* cursor)
	{
		m_CurrentCursor = NUM_CURSORS; // Set current cursor to invalid
		GLFWcursor* cur = static_cast<GLFWcursor*>(cursor);
		glfwSetCursor(m_Window, cur);
	}

	void* WindowsWindow::CreateCustomCursor(uint8_t* pixels, uint32_t width, uint32_t height, int32_t xOffset, int32_t yOffset)
	{
		GLFWimage image;
		image.pixels = pixels;
		image.width = width;
		image.height = height;
		return glfwCreateCursor(&image, xOffset, yOffset);
	}

	void* WindowsWindow::GetWindow() const
	{
		return m_Window;
	}

	void* WindowsWindow::GetNativeWindow() const
	{
		return glfwGetWin32Window(m_Window);
	}


	void WindowsWindow::Destroy()
	{
		glfwDestroyWindow(m_Window);
		glfwTerminate();
	}
}