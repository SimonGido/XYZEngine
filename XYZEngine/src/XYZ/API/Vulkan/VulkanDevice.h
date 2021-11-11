#pragma once
#include "XYZ/Core/Ref.h"

#include "Vulkan.h"

#include <unordered_set>
#include <set>

namespace XYZ {
	// Physical device
	class VulkanPhysicalDevice : public RefCount
	{
	public:
		struct QueueFamilyIndices
		{
			static constexpr uint32_t Invalid = UINT32_MAX;
			uint32_t Graphics	   = Invalid;
			uint32_t Compute	   = Invalid;
			uint32_t Transfer	   = Invalid;
			uint32_t Presentation  = Invalid;
		};
		struct SwapChainSupportDetails
		{
			VkSurfaceCapabilitiesKHR		Capabilities;
			std::vector<VkSurfaceFormatKHR> Formats;
			std::vector<VkPresentModeKHR>   PresentModes;
		};
	public:
		VulkanPhysicalDevice(VkSurfaceKHR surface);
		~VulkanPhysicalDevice();

		bool							  IsExtensionSupported(const std::string& extensionName) const;
		VkPhysicalDevice				  GetVulkanPhysicalDevice() const { return m_PhysicalDevice; }
		VkPhysicalDeviceMemoryProperties  GetMemoryProperties()		const { return m_MemoryProperties; }
		
		const SwapChainSupportDetails&    GetSwapChainSupportDetails() const { return m_SwapChainSupportDetails; }
		const QueueFamilyIndices&		  GetQueueFamilyIndices()      const { return m_QueueFamilyIndices; }
		const VkPhysicalDeviceProperties& GetProperties()		       const { return m_Properties; }
		const VkPhysicalDeviceFeatures&   GetFeatures()				   const { return m_Features; }
	
	private:
		void setupQueueFamilyIndices(int flags);
		void findPresentationQueue(VkSurfaceKHR surface);
		void createQueuesInfo(const std::set<uint32_t>& familyIndices);
		
	private:
		VkPhysicalDevice					 m_PhysicalDevice;
		VkPhysicalDeviceProperties			 m_Properties;
		VkPhysicalDeviceFeatures			 m_Features;
		VkPhysicalDeviceMemoryProperties	 m_MemoryProperties;

		QueueFamilyIndices					 m_QueueFamilyIndices;
		SwapChainSupportDetails				 m_SwapChainSupportDetails;

		std::unordered_set<std::string>		 m_SupportedExtensions;
		std::vector<VkQueueFamilyProperties> m_QueueFamilyProperties;
		std::vector<VkDeviceQueueCreateInfo> m_QueueCreateInfos;
	
		friend class VulkanDevice;
	};

	// Logical device
	class VulkanDevice : public RefCount
	{
	public:
		VulkanDevice(VkPhysicalDeviceFeatures enabledFeatures);
		~VulkanDevice();

		void Init(VkSurfaceKHR surface);

		void Destroy();
		void FlushCommandBuffer(VkCommandBuffer commandBuffer);
		void FlushCommandBuffer(VkCommandBuffer commandBuffer, VkQueue queue);


		VkQueue GetGraphicsQueue() { return m_GraphicsQueue; }
		VkQueue GetComputeQueue()  { return m_ComputeQueue; }
		VkQueue GetPresentationQueue() { return m_PresentationQueue; }

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
		VkQueue					  m_PresentationQueue;

		VkCommandPool			  m_CommandPool;
		VkCommandPool			  m_ComputeCommandPool;	
		bool					  m_EnableDebugMarkers;
	};
}