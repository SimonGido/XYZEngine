#include "stdafx.h"
#include "VulkanDescriptorAllocator.h"

#include "VulkanContext.h"

#include "XYZ/Debug/Profiler.h"
#include "XYZ/Debug/Timer.h"

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
		{ VK_DESCRIPTOR_TYPE_SAMPLER, 2000 },
		{ VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, 2000 },
		{ VK_DESCRIPTOR_TYPE_SAMPLED_IMAGE, 2000 },
		{ VK_DESCRIPTOR_TYPE_STORAGE_IMAGE, 2000 },
		{ VK_DESCRIPTOR_TYPE_UNIFORM_TEXEL_BUFFER, 2000 },
		{ VK_DESCRIPTOR_TYPE_STORAGE_TEXEL_BUFFER, 2000 },
		{ VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, 2000 },
		{ VK_DESCRIPTOR_TYPE_STORAGE_BUFFER, 2000 },
		{ VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER_DYNAMIC, 2000 },
		{ VK_DESCRIPTOR_TYPE_STORAGE_BUFFER_DYNAMIC, 2000 },
		{ VK_DESCRIPTOR_TYPE_INPUT_ATTACHMENT, 2000 }
	};

	void VulkanDescriptorAllocator::Init()
	{
		XYZ_ASSERT(!m_Initialized, "Vulkan Descriptor Allocator is already initialized");
		m_Initialized = true;
		m_AllocatorVersion = 0;
		Ref<VulkanDescriptorAllocator> instance = this;
		Renderer::Submit([instance]() mutable 
		{
			std::scoped_lock<std::mutex> lock(instance->m_PoolMutex);
			const uint32_t framesInFlight = Renderer::GetConfiguration().FramesInFlight;	
			instance->m_Allocators.resize(framesInFlight);
			for (uint32_t frame = 0; frame < framesInFlight; ++frame)
			{
				instance->m_Allocators[frame].InUsePool = instance->createPool();
			}
		});
	}

	void VulkanDescriptorAllocator::Shutdown()
	{
		Ref<VulkanDescriptorAllocator> instance = this;
		Renderer::SubmitResource([instance]() mutable {
			std::scoped_lock<std::mutex> lock(instance->m_PoolMutex);
			VkDevice device = VulkanContext::GetCurrentDevice()->GetVulkanDevice();

			for (auto& allocator : instance->m_Allocators)
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
	VkDescriptorSet VulkanDescriptorAllocator::RT_Allocate(VkDescriptorSetAllocateInfo& allocInfo)
	{
		XYZ_PROFILE_FUNC("VulkanDescriptorAllocator::RT_Allocate");
		return RT_allocate(allocInfo);
	}
	VkDescriptorSet VulkanDescriptorAllocator::RT_Allocate(const VkDescriptorSetLayout& layout)
	{
		XYZ_PROFILE_FUNC("VulkanDescriptorAllocator::RT_Allocate");
		const uint32_t frame = Renderer::GetCurrentFrame();
		VkDescriptorSetAllocateInfo allocInfo;
		VkDevice device = VulkanContext::GetCurrentDevice()->GetVulkanDevice();

		VkDescriptorSet resultDescr;
		allocInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO;
		allocInfo.pNext = nullptr;
		allocInfo.descriptorSetCount = 1;
		allocInfo.pSetLayouts = &layout;

		return RT_allocate(allocInfo);
	}
	void VulkanDescriptorAllocator::TryResetFull()
	{
		Ref<VulkanDescriptorAllocator> instance = this;
		Renderer::SubmitResource([instance]() mutable {
			const uint32_t frame = Renderer::GetCurrentFrame();
			instance->RT_TryResetFull(frame);

		});
	}
	void VulkanDescriptorAllocator::RT_TryResetFull(uint32_t frame)
	{
		XYZ_PROFILE_FUNC("VulkanDescriptorAllocator::RT_TryResetFull");
		VkDevice device = VulkanContext::GetCurrentDevice()->GetVulkanDevice();
		{
			std::scoped_lock<std::mutex> poolLock(m_PoolMutex);
			auto& fullPools = m_Allocators[frame].FullPools;
			if (fullPools.size() >= sc_AutoResetCount)
			{
				XYZ_PROFILE_FUNC("VulkanDescriptorAllocator::RT_TryResetFull - while loop");
				while (fullPools.size() != 1)
				{
					XYZ_PROFILE_FUNC("VulkanDescriptorAllocator::vkResetDescriptorPool");
					VkDescriptorPool pool = fullPools.front();
					fullPools.pop_front();
					VK_CHECK_RESULT(vkResetDescriptorPool(device, pool, VkDescriptorPoolResetFlags{ 0 }));
					m_Allocators[frame].ReusablePools.push_back(pool);
				}
				std::scoped_lock<std::mutex> versionLock(m_VersionMutex);
				m_AllocatorVersion++;
			}
		}
	}
	VulkanDescriptorAllocator::Version VulkanDescriptorAllocator::GetVersion() const
	{
		std::scoped_lock<std::mutex> versionLock(m_VersionMutex);
		return m_AllocatorVersion;
	}
	VkDescriptorPool VulkanDescriptorAllocator::createPool() const
	{
		XYZ_PROFILE_FUNC("VulkanDescriptorAllocator::createPool");
		VkDescriptorPool result;
		VkDescriptorPoolCreateInfo pool_info = {};
		pool_info.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO;
		pool_info.flags = 0;// VK_DESCRIPTOR_POOL_CREATE_FREE_DESCRIPTOR_SET_BIT;
		pool_info.maxSets = sc_MaxSets;
		pool_info.poolSizeCount = std::size(s_PoolSizes);
		pool_info.pPoolSizes = s_PoolSizes;
		VkDevice device = VulkanContext::GetCurrentDevice()->GetVulkanDevice();

		VK_CHECK_RESULT(vkCreateDescriptorPool(device, &pool_info, nullptr, &result));
		return result;
	}
	VkDescriptorSet VulkanDescriptorAllocator::RT_allocate(VkDescriptorSetAllocateInfo& allocInfo)
	{
		XYZ_PROFILE_FUNC("VulkanDescriptorAllocator::RT_allocate");
		const uint32_t frame = Renderer::GetCurrentFrame();
		{
			std::scoped_lock<std::mutex> lock(m_PoolMutex);
			VkDevice device = VulkanContext::GetCurrentDevice()->GetVulkanDevice();
			allocInfo.descriptorPool = m_Allocators[frame].InUsePool;
			VkDescriptorSet resultDescr;
			VkResult result;
			{
				XYZ_PROFILE_FUNC("VulkanDescriptorAllocator::vkAllocateDescriptorSets");
				result = vkAllocateDescriptorSets(device, &allocInfo, &resultDescr);
				if (result == VK_SUCCESS)
					return resultDescr;
			}
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
		return RT_allocate(allocInfo);
	}
	bool VulkanDescriptorAllocator::getReusablePool(VkDescriptorPool& pool, uint32_t frame)
	{
		XYZ_PROFILE_FUNC("VulkanDescriptorAllocator::getReusablePool");
		if (!m_Allocators[frame].ReusablePools.empty())
		{
			pool = m_Allocators[frame].ReusablePools.back();
			m_Allocators[frame].ReusablePools.pop_back();
			return true;
		}
		return false;
	}

}