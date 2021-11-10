#pragma once

#include "Vulkan.h"

#include <vector>

struct GLFWwindow;
namespace XYZ {

	class VulkanSwapChain
	{
	public:
		VulkanSwapChain() = default;

		void Init(VkInstance instance);
		void InitSurface(GLFWwindow* windowHandle);


	private:
		VkInstance m_Instance;
	};
}