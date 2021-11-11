#include "stdafx.h"
#include "VulkanSwapChain.h"

#include <GLFW/glfw3.h>

namespace XYZ {

	// Macro to get a procedure address based on a vulkan instance
	#define GET_INSTANCE_PROC_ADDR(inst, entrypoint)                        \
	{                                                                       \
		fp##entrypoint = reinterpret_cast<PFN_vk##entrypoint>(vkGetInstanceProcAddr(inst, "vk"#entrypoint)); \
		XYZ_ASSERT(fp##entrypoint, "");                                     \
	}

	// Macro to get a procedure address based on a vulkan device
	#define GET_DEVICE_PROC_ADDR(dev, entrypoint)                           \
	{                                                                       \
		fp##entrypoint = reinterpret_cast<PFN_vk##entrypoint>(vkGetDeviceProcAddr(dev, "vk"#entrypoint));   \
		XYZ_ASSERT(fp##entrypoint, "");                                     \
	}


	static PFN_vkGetPhysicalDeviceSurfaceCapabilitiesKHR    fpGetPhysicalDeviceSurfaceCapabilitiesKHR;
	static PFN_vkGetPhysicalDeviceSurfaceFormatsKHR			fpGetPhysicalDeviceSurfaceFormatsKHR;
	static PFN_vkGetPhysicalDeviceSurfacePresentModesKHR	fpGetPhysicalDeviceSurfacePresentModesKHR;
	static PFN_vkCreateSwapchainKHR							fpCreateSwapchainKHR;
	static PFN_vkDestroySwapchainKHR						fpDestroySwapchainKHR;
	static PFN_vkGetSwapchainImagesKHR						fpGetSwapchainImagesKHR;
	static PFN_vkAcquireNextImageKHR						fpAcquireNextImageKHR;
	static PFN_vkQueuePresentKHR							fpQueuePresentKHR;


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
			fpDestroySwapchainKHR(device, m_SwapChain, nullptr);
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
		GET_DEVICE_PROC_ADDR(vulkanDevice, CreateSwapchainKHR);
		GET_DEVICE_PROC_ADDR(vulkanDevice, DestroySwapchainKHR);
		GET_DEVICE_PROC_ADDR(vulkanDevice, GetSwapchainImagesKHR);
		GET_DEVICE_PROC_ADDR(vulkanDevice, AcquireNextImageKHR);
		GET_DEVICE_PROC_ADDR(vulkanDevice, QueuePresentKHR);

		GET_INSTANCE_PROC_ADDR(m_Instance, GetPhysicalDeviceSurfaceCapabilitiesKHR);
		GET_INSTANCE_PROC_ADDR(m_Instance, GetPhysicalDeviceSurfaceFormatsKHR);
		GET_INSTANCE_PROC_ADDR(m_Instance, GetPhysicalDeviceSurfacePresentModesKHR);
	}
}