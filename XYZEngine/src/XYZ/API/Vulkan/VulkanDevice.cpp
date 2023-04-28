#include "stdafx.h"
#include "VulkanDevice.h"

#include "VulkanContext.h"

namespace XYZ {

	struct SwapChainSupportDetails
	{
		VkSurfaceCapabilitiesKHR		Capabilities;
		std::vector<VkSurfaceFormatKHR> Formats;
		std::vector<VkPresentModeKHR>   PresentModes;
	};
	static SwapChainSupportDetails QuerySwapChainSupport(VkSurfaceKHR surface, VkPhysicalDevice physicalDevice)
	{
		SwapChainSupportDetails details;
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
		XYZ_CORE_WARN("Could not find any discrete GPU");
		return devices.back();
	}
	static VkPhysicalDevice FindSuitableGPU(VkSurfaceKHR surface, const std::vector<VkPhysicalDevice>& devices)
	{
		const uint32_t discreteGPUScore = 1000;

		if (devices.empty())
			return VK_NULL_HANDLE;

		VkPhysicalDevice result = VK_NULL_HANDLE;
		
		uint32_t maxScore = 0;
		uint32_t score = 0;
		for (const auto& device : devices)
		{
			VkPhysicalDeviceProperties2 deviceProperties;
			deviceProperties.sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_PROPERTIES_2;
			VkPhysicalDeviceRayTracingPipelinePropertiesKHR rtProperties{ VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_RAY_TRACING_PIPELINE_PROPERTIES_KHR };
			deviceProperties.pNext = &rtProperties;

			vkGetPhysicalDeviceProperties2(device, &deviceProperties);
			SwapChainSupportDetails swapChainDetails = QuerySwapChainSupport(surface, device);

			if (deviceProperties.properties.deviceType == VK_PHYSICAL_DEVICE_TYPE_DISCRETE_GPU)
				score += discreteGPUScore;

			score += deviceProperties.properties.limits.maxImageDimension2D;

			if (score > maxScore 
			&& !swapChainDetails.Formats.empty()
			&& !swapChainDetails.PresentModes.empty())
			{
				maxScore = score;
				result = device;
			}
			score = 0;
		}
		return result;
	}

	VulkanPhysicalDevice::VulkanPhysicalDevice(VkSurfaceKHR surface)
		:
		m_DepthFormat(VK_FORMAT_UNDEFINED),
		m_RaytracingProperties{ VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_RAY_TRACING_PIPELINE_PROPERTIES_KHR }
	{
		const auto vkInstance = VulkanContext::GetInstance();

		const std::vector<VkPhysicalDevice> devices = GetPhysicalDevices(vkInstance);
		m_PhysicalDevice = FindSuitableGPU(surface, devices);
		XYZ_ASSERT(m_PhysicalDevice != VK_NULL_HANDLE, "failed to find GPUs with Vulkan support!");


		m_Properties.sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_PROPERTIES_2;
		m_Properties.pNext = &m_RaytracingProperties;

		vkGetPhysicalDeviceProperties2(m_PhysicalDevice, &m_Properties);

		vkGetPhysicalDeviceFeatures(m_PhysicalDevice, &m_Features);
		vkGetPhysicalDeviceMemoryProperties(m_PhysicalDevice, &m_MemoryProperties);

		m_QueueFamilyProperties = GetQueueFamilyProperties(m_PhysicalDevice);
		m_SupportedExtensions = GetSupportedExtensions(m_PhysicalDevice);

		const int requestedQueueTypes = VK_QUEUE_GRAPHICS_BIT | VK_QUEUE_COMPUTE_BIT | VK_QUEUE_TRANSFER_BIT;
		setupQueueFamilyIndices(requestedQueueTypes);
		findPresentationQueue(surface);
		const std::set<uint32_t> familyIndices{
			m_QueueFamilyIndices.Compute,
			m_QueueFamilyIndices.Graphics,
			m_QueueFamilyIndices.Transfer,
			m_QueueFamilyIndices.Presentation
		};
		createQueuesInfo(familyIndices);
		findDepthFormat();
	}
	VulkanPhysicalDevice::~VulkanPhysicalDevice()
	{	
	}

	bool VulkanPhysicalDevice::IsExtensionSupported(const std::string& extensionName) const
	{
		return m_SupportedExtensions.find(extensionName) != m_SupportedExtensions.end();
	}

	uint32_t VulkanPhysicalDevice::GetMemoryTypeIndex(uint32_t typeBits, VkMemoryPropertyFlags properties) const
	{
		// Iterate over all memory types available for the device used in this example
		for (uint32_t i = 0; i < m_MemoryProperties.memoryTypeCount; i++)
		{
			if ((typeBits & 1) == 1)
			{
				if ((m_MemoryProperties.memoryTypes[i].propertyFlags & properties) == properties)
					return i;
			}
			typeBits >>= 1;
		}

		XYZ_ASSERT(false, "Could not find a suitable memory type!");
		return UINT32_MAX;
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
			vkGetPhysicalDeviceSurfaceSupportKHR(m_PhysicalDevice, i, surface, &supportsPresent[i]);
			//fpGetPhysicalDeviceSurfaceSupportKHR(m_PhysicalDevice, i, surface, &supportsPresent[i]);
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
		for (const auto index : familyIndices)
		{
			VkDeviceQueueCreateInfo queueInfo{};
			queueInfo.sType = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO;
			queueInfo.queueFamilyIndex = index;
			queueInfo.queueCount = 1;
			queueInfo.pQueuePriorities = &m_DefaultQueuePriority;
			m_QueueCreateInfos.push_back(queueInfo);
		}
	}

	void VulkanPhysicalDevice::findDepthFormat()
	{
		// Since all depth formats may be optional, we need to find a suitable depth format to use
		// Start with the highest precision packed format
		const std::vector<VkFormat> depthFormats = {
			VK_FORMAT_D32_SFLOAT_S8_UINT,
			VK_FORMAT_D32_SFLOAT,
			VK_FORMAT_D24_UNORM_S8_UINT,
			VK_FORMAT_D16_UNORM_S8_UINT,
			VK_FORMAT_D16_UNORM
		};

		// TODO: Move to VulkanPhysicalDevice
		for (const auto& format : depthFormats)
		{
			VkFormatProperties formatProps;
			vkGetPhysicalDeviceFormatProperties(m_PhysicalDevice, format, &formatProps);
			// Format must support depth stencil attachment for optimal tiling
			if (formatProps.optimalTilingFeatures & VK_FORMAT_FEATURE_DEPTH_STENCIL_ATTACHMENT_BIT)
			{
				m_DepthFormat = format;
				return;
			}
		}

	}

	VulkanDevice::VulkanDevice(VkPhysicalDeviceFeatures2 enabledFeatures)
		:
		m_LogicalDevice(VK_NULL_HANDLE),
		m_EnabledFeatures(enabledFeatures),
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
		m_PhysicalDevice = Ref<VulkanPhysicalDevice>::Create(surface);
		std::vector<const char*> deviceExtensions;
		// If the device will be used for presenting to a display via a swapchain we need to request the swapchain extension
		XYZ_ASSERT(m_PhysicalDevice->IsExtensionSupported(VK_KHR_SWAPCHAIN_EXTENSION_NAME), "");
		deviceExtensions.push_back(VK_KHR_SWAPCHAIN_EXTENSION_NAME);
		deviceExtensions.push_back("VK_KHR_swapchain");
		deviceExtensions.push_back("VK_KHR_external_memory_win32");
		deviceExtensions.push_back("VK_KHR_external_fence_win32");
		deviceExtensions.push_back("VK_KHR_external_semaphore_win32");
		deviceExtensions.push_back("VK_KHR_get_memory_requirements2");

		
		// Raytracing
		//deviceExtensions.push_back(VK_KHR_ACCELERATION_STRUCTURE_EXTENSION_NAME);
		//deviceExtensions.push_back(VK_KHR_RAY_TRACING_PIPELINE_EXTENSION_NAME);
		//deviceExtensions.push_back(VK_KHR_DEFERRED_HOST_OPERATIONS_EXTENSION_NAME);
		//deviceExtensions.push_back(VK_KHR_BUFFER_DEVICE_ADDRESS_EXTENSION_NAME);
		
		// Storage
		if (m_PhysicalDevice->IsExtensionSupported(VK_KHR_8BIT_STORAGE_EXTENSION_NAME))
			deviceExtensions.push_back(VK_KHR_8BIT_STORAGE_EXTENSION_NAME);
		if (m_PhysicalDevice->IsExtensionSupported(VK_KHR_STORAGE_BUFFER_STORAGE_CLASS_EXTENSION_NAME))
			deviceExtensions.push_back(VK_KHR_STORAGE_BUFFER_STORAGE_CLASS_EXTENSION_NAME);

		if (m_PhysicalDevice->IsExtensionSupported(VK_NV_DEVICE_DIAGNOSTIC_CHECKPOINTS_EXTENSION_NAME))
			deviceExtensions.push_back(VK_NV_DEVICE_DIAGNOSTIC_CHECKPOINTS_EXTENSION_NAME);
		if (m_PhysicalDevice->IsExtensionSupported(VK_NV_DEVICE_DIAGNOSTICS_CONFIG_EXTENSION_NAME))
			deviceExtensions.push_back(VK_NV_DEVICE_DIAGNOSTICS_CONFIG_EXTENSION_NAME);
		


		VkPhysicalDeviceAccelerationStructureFeaturesKHR accelFeature;
		memset(&accelFeature, 0, sizeof(accelFeature));
		accelFeature.sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_ACCELERATION_STRUCTURE_FEATURES_KHR;
		accelFeature.accelerationStructure = true;
		accelFeature.pNext = NULL;

		
		VkPhysicalDeviceRayTracingPipelineFeaturesKHR rtPipelineFeature;
		memset(&rtPipelineFeature, 0, sizeof(rtPipelineFeature));
		rtPipelineFeature.sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_RAY_TRACING_PIPELINE_FEATURES_KHR;
		rtPipelineFeature.rayTracingPipeline = true;
		rtPipelineFeature.pNext = &accelFeature;

		VkPhysicalDeviceVulkan12Features features12;
		memset(&features12, 0, sizeof(VkPhysicalDeviceVulkan12Features));
		features12.sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_VULKAN_1_2_FEATURES;
		features12.bufferDeviceAddress = true;
		features12.pNext = &rtPipelineFeature;
		features12.uniformAndStorageBuffer8BitAccess = true;
		features12.storageBuffer8BitAccess = true;
		features12.shaderInt8 = true;

		VkDeviceCreateInfo deviceCreateInfo = {};
		deviceCreateInfo.sType = VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO;
		m_EnabledFeatures.pNext = &features12;


		deviceCreateInfo.queueCreateInfoCount = static_cast<uint32_t>(m_PhysicalDevice->m_QueueCreateInfos.size());;
		deviceCreateInfo.pQueueCreateInfos = m_PhysicalDevice->m_QueueCreateInfos.data();
		deviceCreateInfo.pNext = &m_EnabledFeatures;

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
		VK_CHECK_RESULT(vkCreateDevice(m_PhysicalDevice->GetVulkanPhysicalDevice(), &deviceCreateInfo, nullptr, &m_LogicalDevice));
		createCommandPools();
		getQueues();
	}
	void VulkanDevice::Destroy() const
	{
		vkDestroyCommandPool(m_LogicalDevice, m_CommandPool, nullptr);
		vkDestroyCommandPool(m_LogicalDevice, m_ComputeCommandPool, nullptr);

		vkDeviceWaitIdle(m_LogicalDevice); // Waiting for queues to finish
		vkDestroyDevice(m_LogicalDevice, nullptr);
	}
	void VulkanDevice::FlushCommandBuffer(VkCommandBuffer commandBuffer) const
	{
		FlushCommandBuffer(commandBuffer, m_GraphicsQueue);
	}
	void VulkanDevice::FlushCommandBuffer(VkCommandBuffer commandBuffer, VkQueue queue) const
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
	VkCommandBuffer VulkanDevice::CreateSecondaryCommandBuffer() const
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