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
		Renderer::Submit([this]() {
			std::scoped_lock<std::mutex> lock(m_PoolMutex);
			const uint32_t framesInFlight = Renderer::GetConfiguration().FramesInFlight;	
			m_InUsePools.resize(framesInFlight);
			for (uint32_t frame = 0; frame < framesInFlight; ++frame)
			{
				m_InUsePools[frame] = createPool();
			}
		});
	}

	void VulkanDescriptorAllocator::Shutdown()
	{
		std::vector<VkDescriptorPool> inUsePools;
		std::vector<VkDescriptorPool> fullPools;
		std::vector<VkDescriptorPool> reusablePools;
		{
			std::scoped_lock<std::mutex> lock(m_PoolMutex);
			inUsePools = std::move(m_InUsePools);
			fullPools = std::move(m_FullPools);
			reusablePools = std::move(m_ReusablePools);
		}
		Renderer::Submit([inUsePools, fullPools, reusablePools]() {
			VkDevice device = VulkanContext::GetCurrentDevice()->GetVulkanDevice();

			for (const auto& pool : inUsePools)
			{
				vkDestroyDescriptorPool(device, pool, nullptr);
			}
			for (const auto& pool : fullPools)
			{
				vkDestroyDescriptorPool(device, pool, nullptr);
			}
			for (const auto& pool : reusablePools)
			{
				vkDestroyDescriptorPool(device, pool, nullptr);
			}
		});
	}
	VkDescriptorSet VulkanDescriptorAllocator::RT_Allocate(const VkDescriptorSetLayout& layout)
	{
		{
			std::scoped_lock<std::mutex> lock(m_PoolMutex);
			uint32_t currentFrame = Renderer::GetAPIContext()->GetCurrentFrame();
			VkDevice device = VulkanContext::GetCurrentDevice()->GetVulkanDevice();

			VkDescriptorSet resultDescr;
			VkDescriptorSetAllocateInfo allocInfo;
			allocInfo.pNext = nullptr;
			allocInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO;
			allocInfo.descriptorPool = m_InUsePools[currentFrame];
			allocInfo.descriptorSetCount = 1;
			allocInfo.pSetLayouts = &layout;

			VkResult result = vkAllocateDescriptorSets(device, &allocInfo, &resultDescr);
			if (result == VK_SUCCESS)
				return resultDescr;

			//we reallocate pools on memory error
			if (IsMemoryError(result))
			{
				m_FullPools.push_back(m_InUsePools[currentFrame]);
				// Find reusable pool if exists
				if (!getReusablePool(m_InUsePools[currentFrame]))
					m_InUsePools[currentFrame] = createPool();
			}
			else
			{
				Utils::VulkanCheckResult(result);
			}
		}
		return RT_Allocate(layout);
	}
	void VulkanDescriptorAllocator::RT_ResetFull()
	{
		VkDevice device = VulkanContext::GetCurrentDevice()->GetVulkanDevice();
		{
			std::scoped_lock<std::mutex> lock(m_PoolMutex);
			for (const auto& pool : m_FullPools)
			{
				vkResetDescriptorPool(device, pool, VkDescriptorPoolResetFlags{ 0 });
				m_ReusablePools.push_back(pool);
			}
			m_FullPools.clear();
		}
	}
	VkDescriptorPool VulkanDescriptorAllocator::createPool() const
	{
		VkDescriptorPool result;
		VkDescriptorPoolCreateInfo pool_info = {};
		pool_info.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO;
		pool_info.flags = 0;
		pool_info.maxSets = sc_MaxSets;
		pool_info.poolSizeCount = std::size(s_PoolSizes);
		pool_info.pPoolSizes = s_PoolSizes;
		VkDevice device = VulkanContext::GetCurrentDevice()->GetVulkanDevice();

		VK_CHECK_RESULT(vkCreateDescriptorPool(device, &pool_info, nullptr, &result));
		return result;
	}
	bool VulkanDescriptorAllocator::getReusablePool(VkDescriptorPool& pool)
	{
		if (!m_ReusablePools.empty())
		{
			pool = m_ReusablePools.back();
			m_ReusablePools.pop_back();
			return true;
		}
		return false;
	}
}