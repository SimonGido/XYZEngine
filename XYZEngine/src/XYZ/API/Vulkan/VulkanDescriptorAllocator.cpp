#include "stdafx.h"
#include "VulkanDescriptorAllocator.h"

#include "VulkanContext.h"

namespace XYZ {
	static bool IsMemoryError(VkResult errorResult) 
	{
		switch (errorResult) 
		{
		case VK_ERROR_FRAGMENTED_POOL:
		case VK_ERROR_OUT_OF_POOL_MEMORY:
			return true;
		}
		return false;
	}
	static VkDescriptorPoolSize s_PoolSizes[] =
	{
		{ VK_DESCRIPTOR_TYPE_SAMPLER, 1000 },
		{ VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, 1000 },
		{ VK_DESCRIPTOR_TYPE_SAMPLED_IMAGE, 1000 },
		{ VK_DESCRIPTOR_TYPE_STORAGE_IMAGE, 1000 },
		{ VK_DESCRIPTOR_TYPE_UNIFORM_TEXEL_BUFFER, 1000 },
		{ VK_DESCRIPTOR_TYPE_STORAGE_TEXEL_BUFFER, 1000 },
		{ VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, 1000 },
		{ VK_DESCRIPTOR_TYPE_STORAGE_BUFFER, 1000 },
		{ VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER_DYNAMIC, 1000 },
		{ VK_DESCRIPTOR_TYPE_STORAGE_BUFFER_DYNAMIC, 1000 },
		{ VK_DESCRIPTOR_TYPE_INPUT_ATTACHMENT, 1000 }
	};

	void VulkanDescriptorAllocator::Init()
	{
		XYZ_ASSERT(!m_Initialized, "Vulkan Descriptor Allocator is already initialized");
		m_Initialized = true;
		Renderer::Submit([this]() 
		{
			std::scoped_lock<std::mutex> lock(m_PoolMutex);
			const uint32_t framesInFlight = Renderer::GetConfiguration().FramesInFlight;	
			m_Allocators.resize(framesInFlight);
			for (uint32_t frame = 0; frame < framesInFlight; ++frame)
			{
				m_Allocators[frame].InUsePool = createPool();
			}
		});
	}

	void VulkanDescriptorAllocator::Shutdown()
	{
		std::vector<Allocator> allocators;
		{
			std::scoped_lock<std::mutex> lock(m_PoolMutex);
			allocators = std::move(m_Allocators);
		}
		Renderer::SubmitResource([allocators]() {
			VkDevice device = VulkanContext::GetCurrentDevice()->GetVulkanDevice();

			for (auto& allocator : allocators)
			{
				vkDestroyDescriptorPool(device, allocator.InUsePool, nullptr);
				for (auto& pool : allocator.FullPools)
				{
					vkDestroyDescriptorPool(device, pool, nullptr);
				}
				for (auto& pool : allocator.ReusablePools)
				{
					vkDestroyDescriptorPool(device, pool, nullptr);
				}
			}
		});
	}
	VkDescriptorSet VulkanDescriptorAllocator::RT_Allocate(const VkDescriptorSetLayout& layout)
	{
		uint32_t frame = Renderer::GetCurrentFrame();
		RT_TryResetFull(frame);

		{
			std::scoped_lock<std::mutex> lock(m_PoolMutex);		
			VkDevice device = VulkanContext::GetCurrentDevice()->GetVulkanDevice();

			VkDescriptorSet resultDescr;
			VkDescriptorSetAllocateInfo allocInfo;
			allocInfo.pNext = nullptr;
			allocInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO;
			allocInfo.descriptorPool = m_Allocators[frame].InUsePool;
			allocInfo.descriptorSetCount = 1;
			allocInfo.pSetLayouts = &layout;

			VkResult result = vkAllocateDescriptorSets(device, &allocInfo, &resultDescr);
			if (result == VK_SUCCESS)
				return resultDescr;

			//we reallocate pools on memory error
			if (IsMemoryError(result))
			{
				m_Allocators[frame].FullPools.push_back(m_Allocators[frame].InUsePool);
				// Find reusable pool if exists
				if (!getReusablePool(m_Allocators[frame].InUsePool, frame))
					m_Allocators[frame].InUsePool = createPool();
			}
			else
			{
				Utils::VulkanCheckResult(result);
			}
		}
		return RT_Allocate(layout);
	}
	void VulkanDescriptorAllocator::RT_TryResetFull(uint32_t frame)
	{
		VkDevice device = VulkanContext::GetCurrentDevice()->GetVulkanDevice();
		{
			std::scoped_lock<std::mutex> lock(m_PoolMutex);
			auto& fullPools = m_Allocators[frame].FullPools;
			if (fullPools.size() >= sc_AutoResetCount)
			{
				XYZ_WARN("Reseting full VulkanDescriptor pools frame {}", frame);
				while (fullPools.size() != 1)
				{
					VkDescriptorPool pool = fullPools.front();
					fullPools.pop_front();
					VK_CHECK_RESULT(vkResetDescriptorPool(device, pool, VkDescriptorPoolResetFlags{ 0 }));
					m_Allocators[frame].ReusablePools.push_back(pool);
				}
			}
		}
	}
	VkDescriptorPool VulkanDescriptorAllocator::createPool() const
	{
		VkDescriptorPool result;
		VkDescriptorPoolCreateInfo pool_info = {};
		pool_info.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO;
		pool_info.flags = VK_DESCRIPTOR_POOL_CREATE_FREE_DESCRIPTOR_SET_BIT;
		pool_info.maxSets = sc_MaxSets;
		pool_info.poolSizeCount = std::size(s_PoolSizes);
		pool_info.pPoolSizes = s_PoolSizes;
		VkDevice device = VulkanContext::GetCurrentDevice()->GetVulkanDevice();

		VK_CHECK_RESULT(vkCreateDescriptorPool(device, &pool_info, nullptr, &result));
		return result;
	}
	bool VulkanDescriptorAllocator::getReusablePool(VkDescriptorPool& pool, uint32_t frame)
	{
		if (!m_Allocators[frame].ReusablePools.empty())
		{
			pool = m_Allocators[frame].ReusablePools.back();
			m_Allocators[frame].ReusablePools.pop_back();
			return true;
		}
		return false;
	}

}