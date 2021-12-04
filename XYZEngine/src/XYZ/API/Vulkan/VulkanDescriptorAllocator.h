#pragma once
#include "Vulkan.h"

namespace XYZ {
	class VulkanDescriptorAllocator
	{
	public:
		void Init();
		void Shutdown();

		VkDescriptorSet RT_Allocate(const VkDescriptorSetLayout& layout);
		void			RT_ResetFull(uint32_t frame);
	private:
		VkDescriptorPool createPool() const;
		bool			 getReusablePool(VkDescriptorPool& pool, uint32_t frame);
		void			 tryResetFull(uint32_t frame);
	private:
		std::vector<VkDescriptorPool> m_InUsePools;
		vector2D<VkDescriptorPool>	  m_FullPools;
		vector2D<VkDescriptorPool>	  m_ReusablePools;

		bool						  m_Initialized = false;
		std::mutex					  m_PoolMutex;

		static constexpr uint32_t	  sc_MaxSets = 2000;
		static constexpr size_t		  sc_AutoResetCount = 2;
	};
}