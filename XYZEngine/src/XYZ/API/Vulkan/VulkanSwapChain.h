#pragma once
#include "Vulkan.h"
#include "VulkanDevice.h"

#include <vector>

struct GLFWwindow;
namespace XYZ {

	class VulkanSwapChain
	{
	public:
		
	public:
		VulkanSwapChain();
		
		void Destroy();

		void Init(VkInstance instance, const Ref<VulkanDevice>& device);
		void InitSurface(GLFWwindow* windowHandle);

	private:
		void findImageAndColorSpace();

	private:
		VkInstance				m_Instance;
		Ref<VulkanDevice>		m_Device;
		
		VkSurfaceKHR			m_Surface;
		VkSwapchainKHR			m_SwapChain;
	};
}