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
		m_WindowHandle(nullptr),
		m_Instance(VK_NULL_HANDLE),
		m_Surface(VK_NULL_HANDLE),
		m_SwapChain(VK_NULL_HANDLE),
		m_ImageCount(0)
	{
		m_Format.format = VK_FORMAT_MAX_ENUM;
		m_Format.colorSpace = VK_COLOR_SPACE_MAX_ENUM_KHR;
		m_Extent.width = 0;
		m_Extent.height = 0;
	}


	void VulkanSwapChain::Destroy()
	{
		VkDevice device = m_Device->GetVulkanDevice();

		destroySwapChain(m_SwapChain);
		if (m_Surface)
		{
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
		m_WindowHandle = windowHandle;

		m_Device->Init(m_Surface);
		
	}

	void VulkanSwapChain::Create(uint32_t* width, uint32_t* height, bool vSync)
	{
		findSurfaceFormat();
		findImageCount();
		findSwapExtent(width, height);

		VkDevice device = m_Device->GetVulkanDevice();
		const auto& capabilities = m_Device->GetPhysicalDevice()->GetSwapChainSupportDetails().Capabilities;
		VkSurfaceTransformFlagsKHR preTransform;
		if (capabilities.supportedTransforms & VK_SURFACE_TRANSFORM_IDENTITY_BIT_KHR)
		{
			// We prefer a non-rotated transform
			preTransform = VK_SURFACE_TRANSFORM_IDENTITY_BIT_KHR;
		}
		else
		{
			preTransform = capabilities.currentTransform;
		}

		VkSwapchainKHR oldSwapChain = m_SwapChain;

		VkSwapchainCreateInfoKHR createInfo{};
		createInfo.sType = VK_STRUCTURE_TYPE_SWAPCHAIN_CREATE_INFO_KHR;
		createInfo.surface = m_Surface;

		createInfo.minImageCount = m_ImageCount;
		createInfo.imageFormat = m_Format.format;
		createInfo.imageColorSpace = m_Format.colorSpace;
		createInfo.imageExtent = m_Extent;
		createInfo.imageArrayLayers = 1;
		createInfo.imageUsage = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT;
		createInfo.preTransform = (VkSurfaceTransformFlagBitsKHR)preTransform;
		createInfo.compositeAlpha = selectCompositeAlpha();
		createInfo.presentMode    = findPresentMode(vSync);
		createInfo.clipped = VK_TRUE;
		createInfo.oldSwapchain = oldSwapChain;
		VK_CHECK_RESULT(vkCreateSwapchainKHR(device, &createInfo, nullptr, &m_SwapChain));
		destroySwapChain(oldSwapChain);
		getImages();
		createSwapChainBuffers();
	}

	void VulkanSwapChain::findSurfaceFormat()
	{
		const auto& swapChainCapabilities = m_Device->GetPhysicalDevice()->GetSwapChainSupportDetails();
	
		for (const auto& availableFormat : swapChainCapabilities.Formats)
		{
			if (availableFormat.format == VK_FORMAT_B8G8R8A8_SRGB 
		 && availableFormat.colorSpace == VK_COLOR_SPACE_SRGB_NONLINEAR_KHR) 
			{
				m_Format = availableFormat;
				return;
			}
		}
		// preferred combination is not available use first
		m_Format = swapChainCapabilities.Formats[0];
	}

	VkPresentModeKHR VulkanSwapChain::findPresentMode(bool vSync)
	{
		const auto& swapChainCapabilities = m_Device->GetPhysicalDevice()->GetSwapChainSupportDetails();

		if (vSync == false)
		{
			for (const auto& availablePresentMode : swapChainCapabilities.PresentModes)
			{
				if (availablePresentMode == VK_PRESENT_MODE_MAILBOX_KHR)
				{
					return availablePresentMode;
				}
			}
		}
		// Better for mobile phones, less power consumption
		return VK_PRESENT_MODE_FIFO_KHR;
	}

	void VulkanSwapChain::findSwapExtent(uint32_t* width, uint32_t* height)
	{
		const auto& capabilities = m_Device->GetPhysicalDevice()->GetSwapChainSupportDetails().Capabilities;
		if (capabilities.currentExtent.width != UINT32_MAX)
		{
			m_Extent = capabilities.currentExtent;			
		}
		else 
		{
			VkExtent2D actualExtent = {
				*width, *height
			};
			actualExtent.width = std::clamp(actualExtent.width, capabilities.minImageExtent.width, capabilities.maxImageExtent.width);
			actualExtent.height = std::clamp(actualExtent.height, capabilities.minImageExtent.height, capabilities.maxImageExtent.height);
			m_Extent = actualExtent;
		}
		*width  = m_Extent.width;
		*height = m_Extent.height;
	}

	void VulkanSwapChain::findImageCount()
	{
		const auto& capabilities = m_Device->GetPhysicalDevice()->GetSwapChainSupportDetails().Capabilities;
		m_ImageCount = capabilities.minImageCount + 1; // Sometimes we would have to wait on the driver to complete internal operations, so we want min + 1

		// 0 means there is no maximum, we can not exceed it.
		m_ImageCount = std::min(capabilities.maxImageCount, m_ImageCount);
	}

	void VulkanSwapChain::getImages()
	{
		VkDevice device = m_Device->GetVulkanDevice();
		vkGetSwapchainImagesKHR(device, m_SwapChain, &m_ImageCount, nullptr);
		m_Images.resize(m_ImageCount);
		vkGetSwapchainImagesKHR(device, m_SwapChain, &m_ImageCount, m_Images.data());
	}

	void VulkanSwapChain::createSwapChainBuffers()
	{
		VkDevice device = m_Device->GetVulkanDevice();
		m_Buffers.resize(m_Images.size());
		for (size_t i = 0; i < m_Buffers.size(); ++i)
		{
			VkImageViewCreateInfo createInfo{};
			createInfo.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
			createInfo.image = m_Images[i];
			createInfo.viewType = VK_IMAGE_VIEW_TYPE_2D;
			createInfo.format = m_Format.format;
			createInfo.components = {
				VK_COMPONENT_SWIZZLE_R,
				VK_COMPONENT_SWIZZLE_G,
				VK_COMPONENT_SWIZZLE_B,
				VK_COMPONENT_SWIZZLE_A
			};
			createInfo.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
			createInfo.subresourceRange.baseMipLevel = 0;
			createInfo.subresourceRange.levelCount = 1;
			createInfo.subresourceRange.baseArrayLayer = 0;
			createInfo.subresourceRange.layerCount = 1;
			m_Buffers[i].Image = m_Images[i];
			VK_CHECK_RESULT(vkCreateImageView(device, &createInfo, nullptr, &m_Buffers[i].View));
		}
	}


	void VulkanSwapChain::destroySwapChain(VkSwapchainKHR swapChain)
	{
		VkDevice device = m_Device->GetVulkanDevice();
		if (swapChain != VK_NULL_HANDLE)
		{
			for (uint32_t i = 0; i < m_Buffers.size(); i++)
			{
				vkDestroyImageView(device, m_Buffers[i].View, nullptr);
			}
			vkDestroySwapchainKHR(device, swapChain, nullptr);
		}
	}

	VkCompositeAlphaFlagBitsKHR VulkanSwapChain::selectCompositeAlpha() const
	{
		const auto& capabilities = m_Device->GetPhysicalDevice()->GetSwapChainSupportDetails().Capabilities;
		// Simply select the first composite alpha format available
		std::vector<VkCompositeAlphaFlagBitsKHR> compositeAlphaFlags = {
			VK_COMPOSITE_ALPHA_OPAQUE_BIT_KHR,
			VK_COMPOSITE_ALPHA_PRE_MULTIPLIED_BIT_KHR,
			VK_COMPOSITE_ALPHA_POST_MULTIPLIED_BIT_KHR,
			VK_COMPOSITE_ALPHA_INHERIT_BIT_KHR,
		};
		for (auto& compositeAlphaFlag : compositeAlphaFlags)
		{
			if (capabilities.supportedCompositeAlpha & compositeAlphaFlag)
			{
				return compositeAlphaFlag;
			};
		}
		return  compositeAlphaFlags[0];
	}

}