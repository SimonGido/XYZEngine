#pragma once
#include "Vulkan.h"

namespace XYZ {
	class VulkanDescriptorAllocator
	{
	public:
		using Version = uint32_t;

		void Init();
		void Shutdown();

		VkDescriptorSet RT_Allocate(VkDescriptorSetAllocateInfo& allocInfo);
		VkDescriptorSet RT_Allocate(const VkDescriptorSetLayout& layout);
		void			RT_TryResetFull(uint32_t frame);
		Version			GetVersion() const;

	private:
		VkDescriptorPool createPool() const;
		bool			 getReusablePool(VkDescriptorPool& pool, uint32_t frame);

	private:
		struct Allocator
		{
			VkDescriptorPool			  InUsePool;
			std::deque<VkDescriptorPool>  FullPools;
			std::deque<VkDescriptorPool>  ReusablePools;
		};

		std::vector<Allocator>	 m_Allocators;

		bool					  m_Initialized = false;
		std::mutex				  m_PoolMutex;
		mutable std::mutex		  m_VersionMutex;
		Version					  m_AllocatorVersion;

		static constexpr uint32_t sc_MaxSets = 10000;
		static constexpr size_t	  sc_AutoResetCount = 3;
	};
}