#include "stdafx.h"
#include "WindowsWindow.h"
#include "XYZ/Core/Event/Event.h"


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
			data.EventCallback(WindowResizeEvent(width, height));
		});

		glfwSetWindowCloseCallback(m_Window, [](GLFWwindow* window)
		{
			WindowData& data = *(WindowData*)glfwGetWindowUserPointer(window);
			data.EventCallback(WindowCloseEvent());
		});

		glfwSetCharCallback(m_Window, [](GLFWwindow* window, unsigned int key)
		{
			WindowData& data = *(WindowData*)glfwGetWindowUserPointer(window);
			data.EventCallback(KeyTypedEvent(key));
		});

		glfwSetKeyCallback(m_Window, [](GLFWwindow* window, int key, int scancode, int action, int mods)
		{
			WindowData& data = *(WindowData*)glfwGetWindowUserPointer(window);

			switch (action)
			{
			case GLFW_PRESS:
			{
				data.EventCallback(KeyPressedEvent(key, mods));
				break;
			}
			case GLFW_RELEASE:
			{
				data.EventCallback(KeyReleasedEvent(key));
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
				data.EventCallback(MouseButtonPressEvent(button));
				break;
			}
			case GLFW_RELEASE:
			{
				data.EventCallback(MouseButtonReleaseEvent(button));
				break;
			}

			}
		});

		glfwSetScrollCallback(m_Window, [](GLFWwindow* window, double xOffset, double yOffset)
		{
			WindowData& data = *(WindowData*)glfwGetWindowUserPointer(window);
			data.EventCallback(MouseScrollEvent((float)xOffset, (float)yOffset));
		});

		glfwSetCursorPosCallback(m_Window, [](GLFWwindow* window, double xPos, double yPos)
		{
			WindowData& data = *(WindowData*)glfwGetWindowUserPointer(window);
			data.EventCallback(MouseMovedEvent((float)xPos, (float)yPos));
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


	void WindowsWindow::Destroy()
	{
		glfwDestroyWindow(m_Window);
		glfwTerminate();
	}
}