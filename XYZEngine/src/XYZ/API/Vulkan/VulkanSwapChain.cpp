#include "stdafx.h"
#include "VulkanSwapChain.h"

#include <GLFW/glfw3.h>

namespace XYZ {
	void VulkanSwapChain::Init(VkInstance instance)
	{
		m_Instance = instance;
	}
	void VulkanSwapChain::InitSurface(GLFWwindow* windowHandle)
	{
	}
}