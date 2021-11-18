#pragma once
#include "Vulkan.h"

namespace XYZ {
	class VulkanDescriptorAllocator
	{
	public:
		void Init();
		void Shutdown();

		VkDescriptorSet RT_Allocate(const VkDescriptorSetLayout& layout);
		void			RT_ResetFull();
	private:
		VkDescriptorPool createPool() const;
		bool			 getReusablePool(VkDescriptorPool& pool);

	private:
		std::vector<VkDescriptorPool> m_InUsePools;
		std::vector<VkDescriptorPool> m_FullPools;
		std::vector<VkDescriptorPool> m_ReusablePools;

		bool						  m_Initialized = false;
		std::mutex					  m_PoolMutex;

		static constexpr uint32_t	  sc_MaxSets = 2000;
	};
}