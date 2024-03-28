#pragma once
#include "XYZ/Core/Ref/Ref.h"

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

	public:
		VulkanPhysicalDevice(VkSurfaceKHR surface);
		~VulkanPhysicalDevice();

		
		bool							  IsExtensionSupported(const std::string& extensionName) const;
		VkPhysicalDevice				  GetVulkanPhysicalDevice() const { return m_PhysicalDevice; }
		VkFormat						  GetDepthFormat()			const { return m_DepthFormat; }
		uint32_t						  GetMemoryTypeIndex(uint32_t typeBits, VkMemoryPropertyFlags properties) const;
		
	
		const QueueFamilyIndices&		  GetQueueFamilyIndices()	  const { return m_QueueFamilyIndices; }
		const VkPhysicalDeviceProperties& GetProperties()			  const { return m_Properties.properties; }
		const VkPhysicalDeviceFeatures&   GetFeatures()				  const { return m_Features; }
		const VkPhysicalDeviceLimits&     GetLimits()				  const { return m_Properties.properties.limits; }
		const VkPhysicalDeviceMemoryProperties& GetMemoryProperties() const { return m_MemoryProperties; }
		const VkPhysicalDeviceRayTracingPipelinePropertiesKHR& GetRaytracingProperties() const { return m_RaytracingProperties; }
	private:
		void setupQueueFamilyIndices(int flags);
		void findPresentationQueue(VkSurfaceKHR surface);
		void createQueuesInfo(const std::set<uint32_t>& familyIndices);
		void findDepthFormat();

	private:
		VkPhysicalDevice					 m_PhysicalDevice;
		VkPhysicalDeviceProperties2			 m_Properties;
		VkPhysicalDeviceFeatures			 m_Features;
		VkPhysicalDeviceMemoryProperties	 m_MemoryProperties;
		VkFormat							 m_DepthFormat;
		QueueFamilyIndices					 m_QueueFamilyIndices;
		VkPhysicalDeviceRayTracingPipelinePropertiesKHR m_RaytracingProperties;

		std::unordered_set<std::string>		 m_SupportedExtensions;
		std::vector<VkQueueFamilyProperties> m_QueueFamilyProperties;
		std::vector<VkDeviceQueueCreateInfo> m_QueueCreateInfos;
	

		const float m_DefaultQueuePriority = 0.0f;

		friend class VulkanDevice;
	};

	// Logical device
	class VulkanDevice : public RefCount
	{
	public:
		VulkanDevice(VkPhysicalDeviceFeatures2 enabledFeatures);
		virtual ~VulkanDevice() override;

		void Init(VkSurfaceKHR surface);

		void Destroy() const;
		void FlushCommandBuffer(VkCommandBuffer commandBuffer) const;
		void FlushCommandBuffer(VkCommandBuffer commandBuffer, VkQueue queue) const;


		VkQueue GetGraphicsQueue() const { return m_GraphicsQueue; }
		VkQueue GetComputeQueue() const { return m_ComputeQueue; }
		VkQueue GetPresentationQueue() const { return m_PresentationQueue; }

		VkCommandBuffer GetCommandBuffer(bool begin, bool compute = false);
		VkCommandBuffer CreateSecondaryCommandBuffer() const;
		VkDevice		GetVulkanDevice() const { return m_LogicalDevice; }

		const Ref<VulkanPhysicalDevice>& GetPhysicalDevice() const { return m_PhysicalDevice; }

	private:
		void getQueues();
		void createCommandPools();

	private:
		VkDevice				  m_LogicalDevice;
		VkPhysicalDeviceFeatures2 m_EnabledFeatures;
		Ref<VulkanPhysicalDevice> m_PhysicalDevice;

		VkQueue					  m_GraphicsQueue;
		VkQueue					  m_ComputeQueue;
		VkQueue					  m_PresentationQueue;

		VkCommandPool			  m_CommandPool;
		VkCommandPool			  m_ComputeCommandPool;	
		bool					  m_EnableDebugMarkers;
	};
}