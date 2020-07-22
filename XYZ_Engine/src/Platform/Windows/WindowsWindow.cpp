#include "stdafx.h"
#include "WindowsWindow.h"



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
		const GLFWvidmode* mode = glfwGetVideoMode(glfwGetPrimaryMonitor());
		int width = mode->width;
		int height = mode->height;

		m_Data.Width = props.Width;
		m_Data.Height = props.Height;
		
		if (props.Flags & WindowFlags::MAXIMIZED)
		{
			glfwWindowHint(GLFW_MAXIMIZED, true);
			m_Window = glfwCreateWindow(width, height, props.Title.c_str(), NULL, NULL);
		}
		else if (props.Flags & WindowFlags::FULLSCREEN)
		{
			m_Window = glfwCreateWindow(width, height, props.Title.c_str(), glfwGetPrimaryMonitor(), NULL);
		}
		else
		{
			m_Window = glfwCreateWindow(props.Width, props.Height, props.Title.c_str(), NULL, NULL);
		}

		m_Context = APIContext::Create(m_Window);
		m_Context->Init();

		glfwSetWindowUserPointer(m_Window, &m_Data);
		SetVSync(true);

		
		glfwSetWindowSizeCallback(m_Window, [](GLFWwindow* window, int width, int height)
		{
			WindowData& data = *(WindowData*)glfwGetWindowUserPointer(window);
			data.Width = width;
			data.Height = height;
			WindowResizeEvent e(width, height);
			data.EventCallback(e);
			data.This->Execute(e);
		});

		glfwSetWindowCloseCallback(m_Window, [](GLFWwindow* window)
		{
			WindowData& data = *(WindowData*)glfwGetWindowUserPointer(window);
			WindowCloseEvent e;
			data.EventCallback(e);
			data.This->Execute(e);
		});

		glfwSetCharCallback(m_Window, [](GLFWwindow* window, unsigned int key)
		{
			WindowData& data = *(WindowData*)glfwGetWindowUserPointer(window);
			KeyComponentdEvent e(key);
			data.EventCallback(e);
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
				data.EventCallback(e);
				data.This->Execute(e);
				break;
			}
			case GLFW_RELEASE:
			{
				KeyReleasedEvent e(key);
				data.EventCallback(e);
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
				data.EventCallback(e);
				data.This->Execute(e);
				break;
			}
			case GLFW_RELEASE:
			{
				MouseButtonReleaseEvent e(button);
				data.EventCallback(e);
				data.This->Execute(e);
				break;
			}

			}
		});

		glfwSetScrollCallback(m_Window, [](GLFWwindow* window, double xOffset, double yOffset)
		{
			WindowData& data = *(WindowData*)glfwGetWindowUserPointer(window);
			MouseScrollEvent e((float)xOffset, (float)yOffset);
			data.EventCallback(e);
			data.This->Execute(e);
		});

		glfwSetCursorPosCallback(m_Window, [](GLFWwindow* window, double xPos, double yPos)
		{
			WindowData& data = *(WindowData*)glfwGetWindowUserPointer(window);
			MouseMovedEvent e((float)xPos, (float)yPos);
			data.EventCallback(e);
			data.This->Execute(e);
		});
	}

	WindowsWindow::~WindowsWindow()
	{
	}

	void WindowsWindow::Update()
	{
		glfwPollEvents();
		m_Context->SwapBuffers();
	}

	void WindowsWindow::SetVSync(bool enabled)
	{
		glfwSwapInterval(enabled);
	}

	bool WindowsWindow::IsClosed()
	{
		return glfwWindowShouldClose(m_Window);
	}

	void WindowsWindow::SetCursor(WindowCursor cursor)
	{
		if (cursor != m_Current)
		{
			m_Current = cursor;
			if (m_Cursor)
				glfwDestroyCursor(m_Cursor);

			int casted = static_cast<std::underlying_type_t<WindowCursor>>(cursor);
			m_Cursor = glfwCreateStandardCursor(casted);
			glfwSetCursor(m_Window, m_Cursor);
		}
	}


	void WindowsWindow::Destroy()
	{
		glfwDestroyWindow(m_Window);
		glfwTerminate();
	}
}