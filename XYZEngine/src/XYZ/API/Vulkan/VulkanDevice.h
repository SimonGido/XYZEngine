#pragma once
#include "XYZ/Core/Ref.h"

#include "Vulkan.h"

#include <unordered_set>

namespace XYZ {
	// Physical device
	class VulkanPhysicalDevice : public RefCount
	{
	public:
		struct QueueFamilyIndices
		{
			static constexpr int32_t Invalid = -1;
			int32_t Graphics = Invalid;
			int32_t Compute  = Invalid;
			int32_t Transfer = Invalid;
		};

	public:
		VulkanPhysicalDevice();
		~VulkanPhysicalDevice();

		bool							  IsExtensionSupported(const std::string& extensionName) const;
		VkPhysicalDevice				  GetVulkanPhysicalDevice() const { return m_PhysicalDevice; }
		VkPhysicalDeviceMemoryProperties  GetMemoryProperties()		const { return m_MemoryProperties; }
		const QueueFamilyIndices&		  GetQueueFamilyIndices()   const { return m_QueueFamilyIndices; }
		const VkPhysicalDeviceProperties& GetProperties()		    const { return m_Properties; }
		const VkPhysicalDeviceFeatures&   GetFeatures()				const { return m_Features; }
	private:
		void setupQueueFamilyIndices(int flags);
		void createQueuesInfo(int flags);

	private:
		VkPhysicalDevice					 m_PhysicalDevice;
		VkPhysicalDeviceProperties			 m_Properties;
		VkPhysicalDeviceFeatures			 m_Features;
		QueueFamilyIndices					 m_QueueFamilyIndices;
		VkPhysicalDeviceMemoryProperties	 m_MemoryProperties;
		std::unordered_set<std::string>		 m_SupportedExtensions;
		std::vector<VkQueueFamilyProperties> m_QueueFamilyProperties;
		std::vector<VkDeviceQueueCreateInfo> m_QueueCreateInfos;
	
		friend class VulkanDevice;
	};

	// Logical device
	class VulkanDevice : public RefCount
	{
	public:
		VulkanDevice(const Ref<VulkanPhysicalDevice>& physicalDevice, VkPhysicalDeviceFeatures enabledFeatures);
		~VulkanDevice();

		void Destroy();
		void FlushCommandBuffer(VkCommandBuffer commandBuffer);
		void FlushCommandBuffer(VkCommandBuffer commandBuffer, VkQueue queue);


		VkQueue GetGraphicsQueue() { return m_GraphicsQueue; }
		VkQueue GetComputeQueue()  { return m_ComputeQueue; }
		VkCommandBuffer GetCommandBuffer(bool begin, bool compute = false);
		VkCommandBuffer CreateSecondaryCommandBuffer();
		VkDevice		GetVulkanDevice() const { return m_LogicalDevice; }

		const Ref<VulkanPhysicalDevice>& GetPhysicalDevice() const { return m_PhysicalDevice; }

	private:
		void getQueues();
		void createCommandPools();

	private:
		VkDevice				  m_LogicalDevice;
		VkPhysicalDeviceFeatures  m_EnabledFeatures;
		Ref<VulkanPhysicalDevice> m_PhysicalDevice;

		VkQueue					  m_GraphicsQueue;
		VkQueue					  m_ComputeQueue;
		VkCommandPool			  m_CommandPool;
		VkCommandPool			  m_ComputeCommandPool;	
		bool					  m_EnableDebugMarkers;
	};
}