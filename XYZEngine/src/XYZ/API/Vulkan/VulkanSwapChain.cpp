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


	static PFN_vkGetPhysicalDeviceSurfaceSupportKHR			fpGetPhysicalDeviceSurfaceSupportKHR;
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
		m_SwapChain(VK_NULL_HANDLE),
		m_GraphicsQueueNodeIndex(UINT32_MAX)
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
		m_Device = device;
		VkDevice vulkanDevice = m_Device->GetVulkanDevice();
		GET_DEVICE_PROC_ADDR(vulkanDevice, CreateSwapchainKHR);
		GET_DEVICE_PROC_ADDR(vulkanDevice, DestroySwapchainKHR);
		GET_DEVICE_PROC_ADDR(vulkanDevice, GetSwapchainImagesKHR);
		GET_DEVICE_PROC_ADDR(vulkanDevice, AcquireNextImageKHR);
		GET_DEVICE_PROC_ADDR(vulkanDevice, QueuePresentKHR);

		GET_INSTANCE_PROC_ADDR(instance, GetPhysicalDeviceSurfaceSupportKHR);
		GET_INSTANCE_PROC_ADDR(instance, GetPhysicalDeviceSurfaceCapabilitiesKHR);
		GET_INSTANCE_PROC_ADDR(instance, GetPhysicalDeviceSurfaceFormatsKHR);
		GET_INSTANCE_PROC_ADDR(instance, GetPhysicalDeviceSurfacePresentModesKHR);
	}
	void VulkanSwapChain::InitSurface(GLFWwindow* windowHandle)
	{
		VkPhysicalDevice physicalDevice = m_Device->GetPhysicalDevice()->GetVulkanPhysicalDevice();
		
		VK_CHECK_RESULT(glfwCreateWindowSurface(m_Instance, windowHandle, nullptr, &m_Surface));

	
		m_GraphicsQueueNodeIndex = findGraphicsQueueNodeIndex(physicalDevice);
	}
	uint32_t VulkanSwapChain::findGraphicsQueueNodeIndex(VkPhysicalDevice physicalDevice) const
	{
		// Get available queue family properties
		std::vector<VkQueueFamilyProperties> queueProps = GetQueueFamilyProperties(physicalDevice);
		// Iterate over each queue to learn whether it supports presenting:
		// Find a queue with present support
		// Will be used to present the swap chain images to the windowing system
		std::vector<VkBool32> supportsPresent(queueProps.size());
		for (uint32_t i = 0; i < queueProps.size(); i++)
		{
			fpGetPhysicalDeviceSurfaceSupportKHR(physicalDevice, i, m_Surface, &supportsPresent[i]);
		}
		// Search for a graphics and a present queue in the array of queue
		// families, try to find one that supports both
		uint32_t graphicsQueueNodeIndex = UINT32_MAX;
		uint32_t presentQueueNodeIndex = UINT32_MAX;
		for (uint32_t i = 0; i < queueProps.size(); i++)
		{
			if ((queueProps[i].queueFlags & VK_QUEUE_GRAPHICS_BIT) != 0)
			{
				if (graphicsQueueNodeIndex == UINT32_MAX)
				{
					graphicsQueueNodeIndex = i;
				}

				if (supportsPresent[i] == VK_TRUE)
				{
					graphicsQueueNodeIndex = i;
					presentQueueNodeIndex = i;
					break;
				}
			}
		}
		if (presentQueueNodeIndex == UINT32_MAX)
		{
			// If there's no queue that supports both present and graphics
			// try to find a separate present queue
			for (uint32_t i = 0; i < queueProps.size(); ++i)
			{
				if (supportsPresent[i] == VK_TRUE)
				{
					presentQueueNodeIndex = i;
					break;
				}
			}
		}

		XYZ_ASSERT(graphicsQueueNodeIndex != UINT32_MAX, "");
		XYZ_ASSERT(presentQueueNodeIndex != UINT32_MAX, "");
		return graphicsQueueNodeIndex;
	}
}