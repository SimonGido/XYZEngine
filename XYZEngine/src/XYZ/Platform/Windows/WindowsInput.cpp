#include "stdafx.h"

#ifdef XYZ_PLATFORM_WINDOWS
#include "XYZ/Core/Input.h"

#include "XYZ/Core/Application.h"
#include <GLFW/glfw3.h>


namespace XYZ {
	bool Input::IsKeyPressed(KeyCode button)
	{
		auto window = static_cast<GLFWwindow*>(Application::Get().GetWindow().GetWindow());
		auto state = glfwGetKey(window, static_cast<int32_t>(button));
		return state == GLFW_PRESS || state == GLFW_REPEAT;
	}

	bool Input::IsMouseButtonPressed(MouseCode button)
	{
		auto window = static_cast<GLFWwindow*>(Application::Get().GetWindow().GetWindow());
		auto state = glfwGetMouseButton(window, static_cast<int32_t>(button));
		return state == GLFW_PRESS;
	}

	std::pair<float, float> Input::GetMousePosition()
	{
		auto window = static_cast<GLFWwindow*>(Application::Get().GetWindow().GetWindow());
		double xpos, ypos;
		glfwGetCursorPos(window, &xpos, &ypos);

		return { (float)xpos, (float)ypos };
	}

	float Input::GetMouseX()
	{
		auto [x, y] = GetMousePosition();
		return x;;
	}

	float Input::GetMouseY()
	{
		auto [x, y] = GetMousePosition();
		return y;
	}

	std::pair<int, int> Input::GetWindowSize()
	{
		int width, height;
		GLFWwindow* window = (GLFWwindow*)Application::Get().GetWindow().GetWindow();
		glfwGetWindowSize(window, &width, &height);
		return std::pair<int, int>(width, height);
	}
	std::pair<int, int> Input::GetWindowPosition()
	{
		int positionX, positionY;
		GLFWwindow* window = (GLFWwindow*)Application::Get().GetWindow().GetWindow();
		glfwGetWindowPos(window, &positionX, &positionY);
		return std::pair<int, int>(positionX, positionY);
	}
}

#endif