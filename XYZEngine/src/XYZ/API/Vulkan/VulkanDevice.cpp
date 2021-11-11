#include "stdafx.h"
#include "VulkanDevice.h"

#include "VulkanContext.h"

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



	static PFN_vkGetPhysicalDeviceSurfaceSupportKHR	fpGetPhysicalDeviceSurfaceSupportKHR;
	

	static std::vector<VkPhysicalDevice> GetPhysicalDevices(VkInstance instance)
	{
		std::vector<VkPhysicalDevice> devices;
		uint32_t deviceCount = 0;
		vkEnumeratePhysicalDevices(instance, &deviceCount, nullptr);
		if (deviceCount)
		{
			devices.resize(deviceCount);
			vkEnumeratePhysicalDevices(instance, &deviceCount, devices.data());
		}
		return devices;
	}

	static std::vector<VkQueueFamilyProperties> GetQueueFamilyProperties(VkPhysicalDevice device)
	{
		std::vector<VkQueueFamilyProperties> result;
		uint32_t queueFamilyCount;
		vkGetPhysicalDeviceQueueFamilyProperties(device, &queueFamilyCount, nullptr);
		XYZ_ASSERT(queueFamilyCount > 0, "");
		result.resize(queueFamilyCount);
		vkGetPhysicalDeviceQueueFamilyProperties(device, &queueFamilyCount, result.data());
		return result;
	}

	static std::unordered_set<std::string> GetSupportedExtensions(VkPhysicalDevice device)
	{
		std::unordered_set<std::string> result;
		uint32_t extCount = 0;
		vkEnumerateDeviceExtensionProperties(device, nullptr, &extCount, nullptr);
		if (extCount > 0)
		{
			std::vector<VkExtensionProperties> extensions(extCount);
			if (vkEnumerateDeviceExtensionProperties(device, nullptr, &extCount, &extensions.front()) == VK_SUCCESS)
			{
				for (const auto& ext : extensions)
				{
					result.emplace(ext.extensionName);
				}
			}
		}
		return result;
	}

	static VkPhysicalDevice	FindDiscreteGPU(const std::vector<VkPhysicalDevice>& devices)
	{
		if (devices.empty())
			return VK_NULL_HANDLE;
		for (const auto& device : devices)
		{
			VkPhysicalDeviceProperties deviceProperties;
			vkGetPhysicalDeviceProperties(device, &deviceProperties);

			if (deviceProperties.deviceType == VK_PHYSICAL_DEVICE_TYPE_DISCRETE_GPU)
				return device;
		}
		XYZ_WARN("Could not find any discrete GPU");
		return devices.back();
	}
	static VkPhysicalDevice FindSuitableGPU(const std::vector<VkPhysicalDevice>& devices)
	{
		const uint32_t discreteGPUScore = 1000;

		if (devices.empty())
			return VK_NULL_HANDLE;

		VkPhysicalDevice result = VK_NULL_HANDLE;
		uint32_t maxScore = 0;
		uint32_t score = 0;
		for (const auto& device : devices)
		{
			VkPhysicalDeviceProperties deviceProperties;
			vkGetPhysicalDeviceProperties(device, &deviceProperties);

			if (deviceProperties.deviceType == VK_PHYSICAL_DEVICE_TYPE_DISCRETE_GPU)
				score += discreteGPUScore;

			score += deviceProperties.limits.maxImageDimension2D;

			if (score > maxScore)
			{
				maxScore = score;
				result = device;
			}
			score = 0;
		}
		return result;
	}

	VulkanPhysicalDevice::VulkanPhysicalDevice()
	{
		auto vkInstance = VulkanContext::GetInstance();

		std::vector<VkPhysicalDevice> devices = GetPhysicalDevices(vkInstance);
		m_PhysicalDevice = FindSuitableGPU(devices);
		XYZ_ASSERT(m_PhysicalDevice != VK_NULL_HANDLE, "failed to find GPUs with Vulkan support!");
		GET_INSTANCE_PROC_ADDR(vkInstance, GetPhysicalDeviceSurfaceSupportKHR);

		vkGetPhysicalDeviceProperties(m_PhysicalDevice, &m_Properties);
		vkGetPhysicalDeviceFeatures(m_PhysicalDevice, &m_Features);
		vkGetPhysicalDeviceMemoryProperties(m_PhysicalDevice, &m_MemoryProperties);

		m_QueueFamilyProperties = GetQueueFamilyProperties(m_PhysicalDevice);
		m_SupportedExtensions = GetSupportedExtensions(m_PhysicalDevice);
	}
	VulkanPhysicalDevice::~VulkanPhysicalDevice()
	{	
	}
	void VulkanPhysicalDevice::Init(VkSurfaceKHR surface)
	{	
		const int requestedQueueTypes = VK_QUEUE_GRAPHICS_BIT | VK_QUEUE_COMPUTE_BIT | VK_QUEUE_TRANSFER_BIT;
		setupQueueFamilyIndices(requestedQueueTypes);
		findPresentationQueue(surface);
		std::set<uint32_t> familyIndices{
			m_QueueFamilyIndices.Compute,
			m_QueueFamilyIndices.Graphics,
			m_QueueFamilyIndices.Transfer,
			m_QueueFamilyIndices.Presentation
		};
		createQueuesInfo(familyIndices);
	}
	bool VulkanPhysicalDevice::IsExtensionSupported(const std::string& extensionName) const
	{
		return m_SupportedExtensions.find(extensionName) != m_SupportedExtensions.end();
	}
	void VulkanPhysicalDevice::setupQueueFamilyIndices(int flags)
	{
		// Dedicated queue for compute
		// Try to find a queue family index that supports compute but not graphics
		if (flags & VK_QUEUE_COMPUTE_BIT)
		{
			for (uint32_t i = 0; i < m_QueueFamilyProperties.size(); i++)
			{
				auto& queueFamilyProperties = m_QueueFamilyProperties[i];
				if ((queueFamilyProperties.queueFlags & VK_QUEUE_COMPUTE_BIT) 
				&& ((queueFamilyProperties.queueFlags & VK_QUEUE_GRAPHICS_BIT) == 0))
				{
					m_QueueFamilyIndices.Compute = i;
					break;
				}
			}
		}

		// Dedicated queue for transfer
		// Try to find a queue family index that supports transfer but not graphics and compute
		if (flags & VK_QUEUE_TRANSFER_BIT)
		{
			for (uint32_t i = 0; i < m_QueueFamilyProperties.size(); i++)
			{
				auto& queueFamilyProperties = m_QueueFamilyProperties[i];
				if ((queueFamilyProperties.queueFlags & VK_QUEUE_TRANSFER_BIT) 
				&& ((queueFamilyProperties.queueFlags & VK_QUEUE_GRAPHICS_BIT) == 0) 
				&& ((queueFamilyProperties.queueFlags & VK_QUEUE_COMPUTE_BIT) == 0))
				{
					m_QueueFamilyIndices.Transfer = i;
					break;
				}
			}
		}

		// For other queue types or if no separate compute queue is present, return the first one to support the requested flags
		for (uint32_t i = 0; i < m_QueueFamilyProperties.size(); i++)
		{
			if ((flags & VK_QUEUE_TRANSFER_BIT) 
			&& m_QueueFamilyIndices.Transfer == QueueFamilyIndices::Invalid)
			{
				if (m_QueueFamilyProperties[i].queueFlags & VK_QUEUE_TRANSFER_BIT)
					m_QueueFamilyIndices.Transfer = i;
			}

			if ((flags & VK_QUEUE_COMPUTE_BIT) 
			&& m_QueueFamilyIndices.Compute == QueueFamilyIndices::Invalid)
			{
				if (m_QueueFamilyProperties[i].queueFlags & VK_QUEUE_COMPUTE_BIT)
					m_QueueFamilyIndices.Compute = i;
			}

			if (flags & VK_QUEUE_GRAPHICS_BIT)
			{
				if (m_QueueFamilyProperties[i].queueFlags & VK_QUEUE_GRAPHICS_BIT)
					m_QueueFamilyIndices.Graphics = i;
			}
		}
		
	}
	void VulkanPhysicalDevice::findPresentationQueue(VkSurfaceKHR surface)
	{
		std::vector<VkBool32> supportsPresent(m_QueueFamilyProperties.size());
		for (uint32_t i = 0; i < m_QueueFamilyProperties.size(); i++)
		{
			fpGetPhysicalDeviceSurfaceSupportKHR(m_PhysicalDevice, i, surface, &supportsPresent[i]);
		}
		// Find queue family that supports both presentation and graphics ( better performance )
		for (uint32_t i = 0; i < m_QueueFamilyProperties.size(); i++)
		{
			if ((m_QueueFamilyProperties[i].queueFlags & VK_QUEUE_GRAPHICS_BIT) != 0)
			{
				if (m_QueueFamilyIndices.Graphics == QueueFamilyIndices::Invalid 
					&& supportsPresent[i] == VK_TRUE)
				{
					m_QueueFamilyIndices.Graphics = i;
					m_QueueFamilyIndices.Presentation = i;
					return;
				}
			}
		}
		
		// If there's no queue that supports both present and graphics
		// try to find a separate present queue
		for (uint32_t i = 0; i < m_QueueFamilyProperties.size(); ++i)
		{
			if (supportsPresent[i] == VK_TRUE)
			{
				m_QueueFamilyIndices.Presentation = i;
				return;
			}
		}
	}
	void VulkanPhysicalDevice::createQueuesInfo(const std::set<uint32_t>& familyIndices)
	{
		const float defaultQueuePriority(0.0f);
		for (auto index : familyIndices)
		{
			VkDeviceQueueCreateInfo queueInfo{};
			queueInfo.sType = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO;
			queueInfo.queueFamilyIndex = index;
			queueInfo.queueCount = 1;
			queueInfo.pQueuePriorities = &defaultQueuePriority;
			m_QueueCreateInfos.push_back(queueInfo);
		}
	}
	
	VulkanDevice::VulkanDevice(const Ref<VulkanPhysicalDevice>& physicalDevice, VkPhysicalDeviceFeatures enabledFeatures)
		:
		m_LogicalDevice(VK_NULL_HANDLE),
		m_EnabledFeatures(enabledFeatures),
		m_PhysicalDevice(physicalDevice),
		m_GraphicsQueue(VK_NULL_HANDLE),
		m_ComputeQueue(VK_NULL_HANDLE),
		m_PresentationQueue(VK_NULL_HANDLE),
		m_CommandPool(VK_NULL_HANDLE),
		m_ComputeCommandPool(VK_NULL_HANDLE),
		m_EnableDebugMarkers(false)
	{
		
	}
	VulkanDevice::~VulkanDevice()
	{
	}
	void VulkanDevice::Init(VkSurfaceKHR surface)
	{
		m_PhysicalDevice->Init(surface);
		std::vector<const char*> deviceExtensions;
		// If the device will be used for presenting to a display via a swapchain we need to request the swapchain extension
		XYZ_ASSERT(m_PhysicalDevice->IsExtensionSupported(VK_KHR_SWAPCHAIN_EXTENSION_NAME), "");
		deviceExtensions.push_back(VK_KHR_SWAPCHAIN_EXTENSION_NAME);

		if (m_PhysicalDevice->IsExtensionSupported(VK_NV_DEVICE_DIAGNOSTIC_CHECKPOINTS_EXTENSION_NAME))
			deviceExtensions.push_back(VK_NV_DEVICE_DIAGNOSTIC_CHECKPOINTS_EXTENSION_NAME);
		if (m_PhysicalDevice->IsExtensionSupported(VK_NV_DEVICE_DIAGNOSTICS_CONFIG_EXTENSION_NAME))
			deviceExtensions.push_back(VK_NV_DEVICE_DIAGNOSTICS_CONFIG_EXTENSION_NAME);

		VkDeviceCreateInfo deviceCreateInfo = {};
		deviceCreateInfo.sType = VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO;

		deviceCreateInfo.queueCreateInfoCount = static_cast<uint32_t>(m_PhysicalDevice->m_QueueCreateInfos.size());;
		deviceCreateInfo.pQueueCreateInfos = m_PhysicalDevice->m_QueueCreateInfos.data();
		deviceCreateInfo.pEnabledFeatures = &m_EnabledFeatures;

		// Enable the debug marker extension if it is present (likely meaning a debugging tool is present)
		if (m_PhysicalDevice->IsExtensionSupported(VK_EXT_DEBUG_MARKER_EXTENSION_NAME))
		{
			deviceExtensions.push_back(VK_EXT_DEBUG_MARKER_EXTENSION_NAME);
			m_EnableDebugMarkers = true;
		}

		if (deviceExtensions.size() > 0)
		{
			deviceCreateInfo.enabledExtensionCount = (uint32_t)deviceExtensions.size();
			deviceCreateInfo.ppEnabledExtensionNames = deviceExtensions.data();
		}
		XYZ_ASSERT(vkCreateDevice(m_PhysicalDevice->GetVulkanPhysicalDevice(), &deviceCreateInfo, nullptr, &m_LogicalDevice) == VK_SUCCESS, "");
		createCommandPools();
		getQueues();
	}
	void VulkanDevice::Destroy()
	{
		vkDestroyCommandPool(m_LogicalDevice, m_CommandPool, nullptr);
		vkDestroyCommandPool(m_LogicalDevice, m_ComputeCommandPool, nullptr);

		vkDeviceWaitIdle(m_LogicalDevice); // Waiting for queues to finish
		vkDestroyDevice(m_LogicalDevice, nullptr);
	}
	void VulkanDevice::FlushCommandBuffer(VkCommandBuffer commandBuffer)
	{
		FlushCommandBuffer(commandBuffer, m_GraphicsQueue);
	}
	void VulkanDevice::FlushCommandBuffer(VkCommandBuffer commandBuffer, VkQueue queue)
	{
		const uint64_t DEFAULT_FENCE_TIMEOUT = 100000000000;

		XYZ_ASSERT(commandBuffer != VK_NULL_HANDLE, "");

		VK_CHECK_RESULT(vkEndCommandBuffer(commandBuffer));

		VkSubmitInfo submitInfo = {};
		submitInfo.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
		submitInfo.commandBufferCount = 1;
		submitInfo.pCommandBuffers = &commandBuffer;

		// Create fence to ensure that the command buffer has finished executing
		VkFenceCreateInfo fenceCreateInfo = {};
		fenceCreateInfo.sType = VK_STRUCTURE_TYPE_FENCE_CREATE_INFO;
		fenceCreateInfo.flags = 0;
		VkFence fence;
		VK_CHECK_RESULT(vkCreateFence(m_LogicalDevice, &fenceCreateInfo, nullptr, &fence));

		// Submit to the queue
		VK_CHECK_RESULT(vkQueueSubmit(queue, 1, &submitInfo, fence));
		// Wait for the fence to signal that command buffer has finished executing
		VK_CHECK_RESULT(vkWaitForFences(m_LogicalDevice, 1, &fence, VK_TRUE, DEFAULT_FENCE_TIMEOUT));

		vkDestroyFence(m_LogicalDevice, fence, nullptr);
		vkFreeCommandBuffers(m_LogicalDevice, m_CommandPool, 1, &commandBuffer);
	}
	VkCommandBuffer VulkanDevice::GetCommandBuffer(bool begin, bool compute)
	{
		VkCommandBuffer cmdBuffer;

		VkCommandBufferAllocateInfo cmdBufAllocateInfo = {};
		cmdBufAllocateInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
		cmdBufAllocateInfo.commandPool = compute ? m_ComputeCommandPool : m_CommandPool;
		cmdBufAllocateInfo.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
		cmdBufAllocateInfo.commandBufferCount = 1;

		VK_CHECK_RESULT(vkAllocateCommandBuffers(m_LogicalDevice, &cmdBufAllocateInfo, &cmdBuffer));

		// If requested, also start the new command buffer
		if (begin)
		{
			VkCommandBufferBeginInfo cmdBufferBeginInfo{};
			cmdBufferBeginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
			VK_CHECK_RESULT(vkBeginCommandBuffer(cmdBuffer, &cmdBufferBeginInfo));
		}

		return cmdBuffer;

	}
	VkCommandBuffer VulkanDevice::CreateSecondaryCommandBuffer()
	{
		VkCommandBuffer cmdBuffer;

		VkCommandBufferAllocateInfo cmdBufAllocateInfo = {};
		cmdBufAllocateInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
		cmdBufAllocateInfo.commandPool = m_CommandPool;
		cmdBufAllocateInfo.level = VK_COMMAND_BUFFER_LEVEL_SECONDARY;
		cmdBufAllocateInfo.commandBufferCount = 1;

		VK_CHECK_RESULT(vkAllocateCommandBuffers(m_LogicalDevice, &cmdBufAllocateInfo, &cmdBuffer));
		return cmdBuffer;

	}
	void VulkanDevice::getQueues()
	{
		vkGetDeviceQueue(m_LogicalDevice, m_PhysicalDevice->m_QueueFamilyIndices.Graphics, 0, &m_GraphicsQueue);
		vkGetDeviceQueue(m_LogicalDevice, m_PhysicalDevice->m_QueueFamilyIndices.Compute, 0, &m_ComputeQueue);
		vkGetDeviceQueue(m_LogicalDevice, m_PhysicalDevice->m_QueueFamilyIndices.Presentation, 0, &m_PresentationQueue);
	}
	void VulkanDevice::createCommandPools()
	{
		VkCommandPoolCreateInfo cmdPoolInfo = {};
		cmdPoolInfo.sType = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO;
		cmdPoolInfo.queueFamilyIndex = m_PhysicalDevice->m_QueueFamilyIndices.Graphics;
		cmdPoolInfo.flags = VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT;
		VK_CHECK_RESULT(vkCreateCommandPool(m_LogicalDevice, &cmdPoolInfo, nullptr, &m_CommandPool));

		cmdPoolInfo.queueFamilyIndex = m_PhysicalDevice->m_QueueFamilyIndices.Compute;
		VK_CHECK_RESULT(vkCreateCommandPool(m_LogicalDevice, &cmdPoolInfo, nullptr, &m_ComputeCommandPool));
	}
}