#include "stdafx.h"
#include "VulkanSwapChain.h"

#include <GLFW/glfw3.h>

namespace XYZ {

	static std::vector<VkQueueFamilyProperties> GetQueueFamilyProperties(VkPhysicalDevice physicalDevice)
	{
		uint32_t queueCount;
		vkGetPhysicalDeviceQueueFamilyProperties(physicalDevice, &queueCount, NULL);
		XYZ_ASSERT(queueCount >= 1, "");

		std::vector<VkQueueFamilyProperties> queueProps(queueCount);
		vkGetPhysicalDeviceQueueFamilyProperties(physicalDevice, &queueCount, queueProps.data());
		return queueProps;
	}

	VulkanSwapChain::VulkanSwapChain()
		:
		m_Instance(VK_NULL_HANDLE),
		m_Surface(VK_NULL_HANDLE),
		m_SwapChain(VK_NULL_HANDLE)
	{
	}


	void VulkanSwapChain::Destroy()
	{
		VkDevice device = m_Device->GetVulkanDevice();

		if (m_SwapChain)
		{
			//for (uint32_t i = 0; i < m_ImageCount; i++)
			//	vkDestroyImageView(device, m_Buffers[i].view, nullptr);
		}
		if (m_Surface)
		{
			vkDestroySwapchainKHR(device, m_SwapChain, nullptr);
			vkDestroySurfaceKHR(m_Instance, m_Surface, nullptr);
		}
		m_Surface = VK_NULL_HANDLE;
		m_SwapChain = VK_NULL_HANDLE;
	}
	void VulkanSwapChain::Init(VkInstance instance, const Ref<VulkanDevice>& device)
	{
		m_Instance = instance;
		m_Device   = device;	
	}
	void VulkanSwapChain::InitSurface(GLFWwindow* windowHandle)
	{
		VK_CHECK_RESULT(glfwCreateWindowSurface(m_Instance, windowHandle, nullptr, &m_Surface));

		m_Device->Init(m_Surface);
		VkPhysicalDevice physicalDevice = m_Device->GetPhysicalDevice()->GetVulkanPhysicalDevice();

		VkDevice vulkanDevice = m_Device->GetVulkanDevice();
	}

	void VulkanSwapChain::findImageAndColorSpace()
	{
		const auto& swapChainCapabilities = m_Device->GetPhysicalDevice()->GetSwapChainSupportDetails();
	}
}