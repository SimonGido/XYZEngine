#include "stdafx.h"
#include "VulkanSwapChain.h"

#include "VulkanRendererAPI.h"
#include "XYZ/Debug/Profiler.h"

#include <GLFW/glfw3.h>

namespace XYZ {

	static VulkanSwapChain::SwapChainSupportDetails QuerySwapChainSupport(VkSurfaceKHR surface, VkPhysicalDevice physicalDevice)
	{
		
		VulkanSwapChain::SwapChainSupportDetails details;
		VK_CHECK_RESULT(vkGetPhysicalDeviceSurfaceCapabilitiesKHR(physicalDevice, surface, &details.Capabilities));

		uint32_t formatCount;
		VK_CHECK_RESULT(vkGetPhysicalDeviceSurfaceFormatsKHR(physicalDevice, surface, &formatCount, nullptr));
		if (formatCount != 0)
		{
			details.Formats.resize(formatCount);
			VK_CHECK_RESULT(vkGetPhysicalDeviceSurfaceFormatsKHR(physicalDevice, surface, &formatCount, details.Formats.data()));
		}

		uint32_t presentModeCount;
		VK_CHECK_RESULT(vkGetPhysicalDeviceSurfacePresentModesKHR(physicalDevice, surface, &presentModeCount, nullptr));

		if (presentModeCount != 0)
		{
			details.PresentModes.resize(presentModeCount);
			VK_CHECK_RESULT(vkGetPhysicalDeviceSurfacePresentModesKHR(physicalDevice, surface, &presentModeCount, details.PresentModes.data()));
		}

		return details;
	}
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
		m_SwapChain(VK_NULL_HANDLE),
		m_VulkanRenderPass(VK_NULL_HANDLE),
		m_Surface(VK_NULL_HANDLE),
		m_ImageCount(0),
		m_CurrentImageIndex(0),
		m_CurrentBufferIndex(0),
		m_CommandPool(VK_NULL_HANDLE),
		m_VSync(false),
		m_RenderPassCreated(false)
	{
		m_Format.format = VK_FORMAT_MAX_ENUM;
		m_Format.colorSpace = VK_COLOR_SPACE_MAX_ENUM_KHR;
		m_Extent.width = 0;
		m_Extent.height = 0;
	}


	void VulkanSwapChain::Destroy()
	{
		m_RenderCommandBuffer.Reset();
		const VkDevice device = m_Device->GetVulkanDevice();
		destroySwapChain(m_SwapChain);
		if (m_Surface)
			vkDestroySurfaceKHR(m_Instance, m_Surface, nullptr);

		for (const auto fence : m_WaitFences)
			vkDestroyFence(device, fence, nullptr);

		for (const auto& sempahore : m_Semaphores)
		{
			vkDestroySemaphore(device, sempahore.PresentComplete, nullptr);
			vkDestroySemaphore(device, sempahore.RenderComplete, nullptr);
		}

		m_Surface = VK_NULL_HANDLE;
		m_SwapChain = VK_NULL_HANDLE;
	}
	void VulkanSwapChain::OnResize(uint32_t width, uint32_t height)
	{
		if (width > 0 && height > 0)
		{	
			const auto device = m_Device->GetVulkanDevice();
			const auto physicaldevice = m_Device->GetPhysicalDevice()->GetVulkanPhysicalDevice();
			VK_CHECK_RESULT(vkGetPhysicalDeviceSurfaceCapabilitiesKHR(physicaldevice, m_Surface, &m_SwapChainDetails.Capabilities));

			vkDeviceWaitIdle(device);
			Create(&width, &height, m_VSync);
			m_CurrentBufferIndex = 0;
			m_CurrentImageIndex = 0;
		}
	}
	void VulkanSwapChain::BeginFrame()
	{
		Renderer::Submit([this]() {

			const auto device = m_Device->GetVulkanDevice();
			
			Renderer::ExecuteResources();
			VK_CHECK_RESULT(vkAcquireNextImageKHR(device, m_SwapChain, UINT64_MAX, m_Semaphores[m_CurrentBufferIndex].PresentComplete, VK_NULL_HANDLE, &m_CurrentImageIndex));
	
		});
	}
	void VulkanSwapChain::Present()
	{
		Renderer::Submit([this]() {

			VkSubmitInfo submitInfo{};
			submitInfo.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;

			const VkPipelineStageFlags waitStages = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
			submitInfo.waitSemaphoreCount = 1;
			submitInfo.pWaitSemaphores = &m_Semaphores[m_CurrentBufferIndex].PresentComplete;
			submitInfo.pWaitDstStageMask = &waitStages;

			submitInfo.pSignalSemaphores = &m_Semaphores[m_CurrentBufferIndex].RenderComplete;
			submitInfo.signalSemaphoreCount = 1;
			submitInfo.pCommandBuffers = &m_CommandBuffers[m_CurrentBufferIndex];
			submitInfo.commandBufferCount = 1;

			VK_CHECK_RESULT(vkResetFences(m_Device->GetVulkanDevice(), 1, &m_WaitFences[m_CurrentBufferIndex]));
			VK_CHECK_RESULT(vkQueueSubmit(m_Device->GetGraphicsQueue(), 1, &submitInfo, m_WaitFences[m_CurrentBufferIndex]));
			const VkResult result = queuePresent(m_Device->GetGraphicsQueue(), m_CurrentImageIndex, m_Semaphores[m_CurrentBufferIndex].RenderComplete);
			if (result != VK_SUCCESS || result == VK_SUBOPTIMAL_KHR)
			{
				if (result == VK_ERROR_OUT_OF_DATE_KHR)
				{
					// Swap chain is no longer compatible with the surface and needs to be recreated
					OnResize(m_Extent.width, m_Extent.height);
					return;
				}
				VK_CHECK_RESULT(result);
			}
			{
				XYZ_PROFILE_FUNC("VulkanSwapChain::Present - WaitForFences");
				m_CurrentBufferIndex = (m_CurrentBufferIndex + 1) % Renderer::GetConfiguration().FramesInFlight;;
				VK_CHECK_RESULT(vkWaitForFences(m_Device->GetVulkanDevice(), 1, &m_WaitFences[m_CurrentBufferIndex], VK_TRUE, UINT64_MAX));
			}
		});
	}
	void VulkanSwapChain::Init(VkInstance instance, const Ref<VulkanDevice>& device)
	{
		m_Instance = instance;
		m_Device = device;
	}
	void VulkanSwapChain::InitSurface(GLFWwindow* windowHandle)
	{
		VK_CHECK_RESULT(glfwCreateWindowSurface(m_Instance, windowHandle, nullptr, &m_Surface));
		m_WindowHandle = windowHandle;

		m_Device->Init(m_Surface);
		m_SwapChainDetails = QuerySwapChainSupport(m_Surface, m_Device->GetPhysicalDevice()->GetVulkanPhysicalDevice());

		createSyncObjects();
		findSurfaceFormat();
		findImageCount();	
	}

	void VulkanSwapChain::Create(uint32_t* width, uint32_t* height, bool vSync)
	{
		const VkDevice device = m_Device->GetVulkanDevice();
		const auto& capabilities = m_SwapChainDetails.Capabilities;
		findSwapExtent(width, height);

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

		const VkSwapchainKHR oldSwapChain = m_SwapChain;

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
		createInfo.presentMode = findPresentMode(vSync);
		createInfo.clipped = VK_TRUE;
		createInfo.oldSwapchain = oldSwapChain;
		VK_CHECK_RESULT(vkCreateSwapchainKHR(device, &createInfo, nullptr, &m_SwapChain));
		destroySwapChain(oldSwapChain);
		getImages();
		createSwapChainBuffers();
		createVulkanRenderPass();
		createFramebuffers();
		createCommandPool();
		createRenderPass();
	}
	Ref<RenderCommandBuffer> VulkanSwapChain::GetRenderCommandBuffer() const
	{
		return Ref<RenderCommandBuffer>(m_RenderCommandBuffer);
	}

	VkFramebuffer VulkanSwapChain::GetFramebuffer(uint32_t index) const
	{
		return m_Framebuffers[index];
	}
	VkFramebuffer VulkanSwapChain::GetCurrentFramebuffer() const
	{
		return m_Framebuffers[m_CurrentImageIndex];
	}

	VkCommandBuffer VulkanSwapChain::GetCommandBuffer(uint32_t frameIndex) const
	{
		XYZ_ASSERT(frameIndex < m_CommandBuffers.size(), "");
		return m_CommandBuffers[frameIndex];
	}

	VkResult VulkanSwapChain::GetFenceStatus(uint32_t frameIndex) const
	{
		const auto device = m_Device->GetVulkanDevice();
		return vkGetFenceStatus(device, m_WaitFences[frameIndex]);
	}

	void VulkanSwapChain::findSurfaceFormat()
	{
		for (const auto& availableFormat : m_SwapChainDetails.Formats)
		{
			if (availableFormat.format == VK_FORMAT_B8G8R8A8_SRGB
				&& availableFormat.colorSpace == VK_COLOR_SPACE_SRGB_NONLINEAR_KHR)
			{
				m_Format = availableFormat;
				return;
			}
		}
		// preferred combination is not available use first
		m_Format = m_SwapChainDetails.Formats[0];
	}

	VkPresentModeKHR VulkanSwapChain::findPresentMode(bool vSync)
	{
		if (vSync == false)
		{
			for (const auto& availablePresentMode : m_SwapChainDetails.PresentModes)
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
		const auto& capabilities = m_SwapChainDetails.Capabilities;
		if (capabilities.currentExtent.width != UINT32_MAX)
		{
			m_Extent = capabilities.currentExtent;
		}
		else
		{
			m_Extent.width  = std::clamp(*width, capabilities.minImageExtent.width, capabilities.maxImageExtent.width);
			m_Extent.height = std::clamp(*height, capabilities.minImageExtent.height, capabilities.maxImageExtent.height);
		}
		*width = m_Extent.width;
		*height = m_Extent.height;
	}

	void VulkanSwapChain::findImageCount()
	{
		const auto& capabilities = m_SwapChainDetails.Capabilities;
		m_ImageCount = capabilities.minImageCount + 1; // Sometimes we would have to wait on the driver to complete internal operations, so we want min + 1

		// 0 means there is no maximum, we can not exceed it.
		m_ImageCount = std::min(capabilities.maxImageCount, m_ImageCount);
	}

	void VulkanSwapChain::getImages()
	{
		const VkDevice device = m_Device->GetVulkanDevice();
		vkGetSwapchainImagesKHR(device, m_SwapChain, &m_ImageCount, nullptr);
		m_Images.resize(m_ImageCount);
		vkGetSwapchainImagesKHR(device, m_SwapChain, &m_ImageCount, m_Images.data());
	}
	void VulkanSwapChain::createSyncObjects()
	{
		m_Semaphores.resize( Renderer::GetConfiguration().FramesInFlight);
		m_WaitFences.resize( Renderer::GetConfiguration().FramesInFlight);

		const VkDevice device = m_Device->GetVulkanDevice();
		VkSemaphoreCreateInfo semaphoreInfo{};
		semaphoreInfo.sType = VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO;
		
		VkFenceCreateInfo fenceCreateInfo{};
		fenceCreateInfo.sType = VK_STRUCTURE_TYPE_FENCE_CREATE_INFO;
		fenceCreateInfo.flags = VK_FENCE_CREATE_SIGNALED_BIT;
		
		
		for (uint32_t i = 0; i <  Renderer::GetConfiguration().FramesInFlight; ++i)
		{
			VK_CHECK_RESULT(vkCreateSemaphore(device, &semaphoreInfo, nullptr, &m_Semaphores[i].PresentComplete));
			VK_CHECK_RESULT(vkCreateSemaphore(device, &semaphoreInfo, nullptr, &m_Semaphores[i].RenderComplete));
			VK_CHECK_RESULT(vkCreateFence(device, &fenceCreateInfo,   nullptr, &m_WaitFences[i]));
		}	
	}
	void VulkanSwapChain::createSwapChainBuffers()
	{
		const VkDevice device = m_Device->GetVulkanDevice();
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
		const VkDevice device = m_Device->GetVulkanDevice();
		if (swapChain != VK_NULL_HANDLE)
		{
			for (const auto framebuffer : m_Framebuffers)
				vkDestroyFramebuffer(device, framebuffer, nullptr);

			vkFreeCommandBuffers(device, m_CommandPool, static_cast<uint32_t>(m_CommandBuffers.size()), m_CommandBuffers.data());
			vkDestroyCommandPool(device, m_CommandPool, nullptr);
			vkDestroyRenderPass(device, m_VulkanRenderPass, nullptr);
			for (uint32_t i = 0; i < m_Buffers.size(); i++)
			{
				vkDestroyImageView(device, m_Buffers[i].View, nullptr);
			}
			vkDestroySwapchainKHR(device, swapChain, nullptr);
		}
	}

	void VulkanSwapChain::createVulkanRenderPass()
	{
		const VkDevice device = m_Device->GetVulkanDevice();
		const VkFormat depthFormat = m_Device->GetPhysicalDevice()->GetDepthFormat();
		// Render Pass
		std::array<VkAttachmentDescription, 2> attachments = {};
		// Color attachment
		attachments[0].format = m_Format.format;
		attachments[0].samples = VK_SAMPLE_COUNT_1_BIT;
		attachments[0].loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
		attachments[0].storeOp = VK_ATTACHMENT_STORE_OP_STORE;
		attachments[0].stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
		attachments[0].stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
		attachments[0].initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
		attachments[0].finalLayout = VK_IMAGE_LAYOUT_PRESENT_SRC_KHR;
		
		// Depth attachment
		attachments[1].format = depthFormat;
		attachments[1].samples = VK_SAMPLE_COUNT_1_BIT;
		attachments[1].loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
		attachments[1].storeOp = VK_ATTACHMENT_STORE_OP_STORE;
		attachments[1].stencilLoadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
		attachments[1].stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
		attachments[1].initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
		attachments[1].finalLayout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;

		VkAttachmentReference colorReference = {};
		colorReference.attachment = 0;
		colorReference.layout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;

		VkAttachmentReference depthReference = {};
		depthReference.attachment = 1;
		depthReference.layout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;

		VkSubpassDescription subpassDescription = {};
		subpassDescription.pipelineBindPoint = VK_PIPELINE_BIND_POINT_GRAPHICS;
		subpassDescription.colorAttachmentCount = 1;
		subpassDescription.pColorAttachments = &colorReference;
		//subpassDescription.pDepthStencilAttachment = &depthReference;
		subpassDescription.inputAttachmentCount = 0;
		subpassDescription.pInputAttachments = nullptr;
		subpassDescription.preserveAttachmentCount = 0;
		subpassDescription.pPreserveAttachments = nullptr;
		subpassDescription.pResolveAttachments = nullptr;

		VkSubpassDependency dependency = {};
		dependency.srcSubpass = VK_SUBPASS_EXTERNAL;
		dependency.dstSubpass = 0;
		dependency.srcStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
		dependency.dstStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
		dependency.srcAccessMask = 0;
		dependency.dstAccessMask = VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT;

		VkRenderPassCreateInfo renderPassInfo = {};
		renderPassInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_CREATE_INFO;
		renderPassInfo.attachmentCount = 1;// static_cast<uint32_t>(attachments.size());
		renderPassInfo.pAttachments = attachments.data();
		renderPassInfo.subpassCount = 1;
		renderPassInfo.pSubpasses = &subpassDescription;
		renderPassInfo.dependencyCount = 1;
		renderPassInfo.pDependencies = &dependency;

		VK_CHECK_RESULT(vkCreateRenderPass(device, &renderPassInfo, nullptr, &m_VulkanRenderPass));
	}
	void VulkanSwapChain::createFramebuffers()
	{
		m_Framebuffers.resize(m_ImageCount);	
		VkFramebufferCreateInfo framebufferCreateInfo{};
		framebufferCreateInfo.sType = VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO;
		framebufferCreateInfo.pNext = NULL;
		framebufferCreateInfo.renderPass = m_VulkanRenderPass;
		framebufferCreateInfo.attachmentCount = 1;
		framebufferCreateInfo.width = m_Extent.width;
		framebufferCreateInfo.height = m_Extent.height;
		framebufferCreateInfo.layers = 1;

		for (size_t i = 0; i < m_Framebuffers.size(); ++i)
		{
			framebufferCreateInfo.pAttachments = &m_Buffers[i].View;
			VK_CHECK_RESULT(vkCreateFramebuffer(m_Device->GetVulkanDevice(), &framebufferCreateInfo, nullptr, &m_Framebuffers[i]));
		}
	}

	void VulkanSwapChain::createCommandPool()
	{
		const VkDevice device = m_Device->GetVulkanDevice();	
		VkCommandPoolCreateInfo cmdPoolInfo = {};
		cmdPoolInfo.sType = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO;
		cmdPoolInfo.queueFamilyIndex = m_Device->GetPhysicalDevice()->GetQueueFamilyIndices().Graphics;
		cmdPoolInfo.flags = VK_COMMAND_POOL_CREATE_TRANSIENT_BIT | VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT;
		VK_CHECK_RESULT(vkCreateCommandPool(device, &cmdPoolInfo, nullptr, &m_CommandPool));

		VkCommandBufferAllocateInfo commandBufferAllocateInfo{};
		commandBufferAllocateInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
		commandBufferAllocateInfo.commandPool = m_CommandPool;
		commandBufferAllocateInfo.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
		const uint32_t count = m_ImageCount;
		commandBufferAllocateInfo.commandBufferCount = count;
		m_CommandBuffers.resize(count);
		VK_CHECK_RESULT(vkAllocateCommandBuffers(device, &commandBufferAllocateInfo, m_CommandBuffers.data()));
	}

	void VulkanSwapChain::createRenderPass()
	{
		if (!m_RenderPassCreated)
		{
			m_RenderCommandBuffer = Ref<VulkanRenderCommandBuffer>::Create("SwapChainCommandBuffer");
			FramebufferSpecification specs;
			specs.Attachments = { ImageFormat::RGBA };
			specs.SwapChainTarget = true;
			specs.Width = m_Extent.width;
			specs.Height = m_Extent.height;

			m_Framebuffer = Ref<VulkanFramebuffer>::Create(specs);
			RenderPassSpecification renderPassSpecs;
			renderPassSpecs.TargetFramebuffer = m_Framebuffer;
			m_RenderPass = Ref<VulkanRenderPass>::Create(renderPassSpecs);
			m_RenderPassCreated = true;
		}
		else
		{
			m_RenderCommandBuffer->m_CommandBuffers = m_CommandBuffers;
			m_Framebuffer->m_RenderPass = m_VulkanRenderPass;
		}
	}

	VkResult VulkanSwapChain::queuePresent(VkQueue queue, uint32_t imageIndex, VkSemaphore waitSemaphore) const
	{
		VkPresentInfoKHR presentInfo = {};
		presentInfo.sType = VK_STRUCTURE_TYPE_PRESENT_INFO_KHR;
		presentInfo.pNext = NULL;
		presentInfo.swapchainCount = 1;
		presentInfo.pSwapchains = &m_SwapChain;
		presentInfo.pImageIndices = &imageIndex;
		// Check if a wait semaphore has been specified to wait for before presenting the image
		if (waitSemaphore != VK_NULL_HANDLE)
		{
			presentInfo.pWaitSemaphores = &waitSemaphore;
			presentInfo.waitSemaphoreCount = 1;
		}

		return vkQueuePresentKHR(queue, &presentInfo);
	}
	VkCompositeAlphaFlagBitsKHR VulkanSwapChain::selectCompositeAlpha() const
	{
		const auto& capabilities = m_SwapChainDetails.Capabilities;
		// Simply select the first composite alpha format available
		const std::vector<VkCompositeAlphaFlagBitsKHR> compositeAlphaFlags = {
			VK_COMPOSITE_ALPHA_OPAQUE_BIT_KHR,
			VK_COMPOSITE_ALPHA_PRE_MULTIPLIED_BIT_KHR,
			VK_COMPOSITE_ALPHA_POST_MULTIPLIED_BIT_KHR,
			VK_COMPOSITE_ALPHA_INHERIT_BIT_KHR,
		};
		for (const auto& compositeAlphaFlag : compositeAlphaFlags)
		{
			if (capabilities.supportedCompositeAlpha & compositeAlphaFlag)
			{
				return compositeAlphaFlag;
			};
		}
		return  compositeAlphaFlags[0];
	}

}