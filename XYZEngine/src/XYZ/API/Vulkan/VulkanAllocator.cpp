#include "stdafx.h"
#include "VulkanAllocator.h"

#include "VulkanContext.h"



namespace XYZ {

	struct VulkanAllocatorData
	{
		VmaAllocator Allocator;
		uint64_t TotalAllocatedBytes = 0;
	};

	struct AllocationCounter
	{
		std::unordered_map<std::string, int64_t> CountMap;
		std::mutex Mutex;
	};

	static VulkanAllocatorData* s_Data = nullptr;

	static AllocationCounter s_AllocationCounter;

	VulkanAllocator::VulkanAllocator(const std::string& tag)
		: m_Tag(tag)
	{
		std::scoped_lock lock(s_AllocationCounter.Mutex);
		if (s_AllocationCounter.CountMap.find(m_Tag) == s_AllocationCounter.CountMap.end())
		{
			s_AllocationCounter.CountMap[m_Tag] = 0;
		}
	}

	VulkanAllocator::~VulkanAllocator()
	{
	}


	VmaAllocation VulkanAllocator::AllocateBuffer(VkBufferCreateInfo bufferCreateInfo, VmaMemoryUsage usage, VkBuffer& outBuffer)
	{
		VmaAllocationCreateInfo allocCreateInfo = {};
		allocCreateInfo.usage = usage;

		VmaAllocation allocation;
		vmaCreateBuffer(s_Data->Allocator, &bufferCreateInfo, &allocCreateInfo, &outBuffer, &allocation, nullptr);


		VmaAllocationInfo allocInfo{};
		vmaGetAllocationInfo(s_Data->Allocator, allocation, &allocInfo);

		s_Data->TotalAllocatedBytes += allocInfo.size;

		increaseAllocationCounter();

		return allocation;
	}

	VmaAllocation VulkanAllocator::AllocateImage(VkImageCreateInfo imageCreateInfo, VmaMemoryUsage usage, VkImage& outImage)
	{
		VmaAllocationCreateInfo allocCreateInfo = {};
		allocCreateInfo.usage = usage;

		VmaAllocation allocation;
		vmaCreateImage(s_Data->Allocator, &imageCreateInfo, &allocCreateInfo, &outImage, &allocation, nullptr);

		// TODO: Tracking
		VmaAllocationInfo allocInfo;
		vmaGetAllocationInfo(s_Data->Allocator, allocation, &allocInfo);
	
		s_Data->TotalAllocatedBytes += allocInfo.size;
		
		increaseAllocationCounter();
		
		return allocation;
	}

	void VulkanAllocator::Free(VmaAllocation allocation)
	{
		vmaFreeMemory(s_Data->Allocator, allocation);
	}

	void VulkanAllocator::DestroyImage(VkImage image, VmaAllocation allocation)
	{
		XYZ_ASSERT(image, "");
		XYZ_ASSERT(allocation, "");
		vmaDestroyImage(s_Data->Allocator, image, allocation);
		decreaseAllocationCounter();
	}

	void VulkanAllocator::DestroyBuffer(VkBuffer buffer, VmaAllocation allocation)
	{
		XYZ_ASSERT(buffer, "");
		XYZ_ASSERT(allocation, "");
		vmaDestroyBuffer(s_Data->Allocator, buffer, allocation);
		decreaseAllocationCounter();
	}

	void VulkanAllocator::UnmapMemory(VmaAllocation allocation)
	{
		vmaUnmapMemory(s_Data->Allocator, allocation);
	}

	void VulkanAllocator::DumpStats()
	{
		const auto& memoryProps = VulkanContext::GetCurrentDevice()->GetPhysicalDevice()->GetMemoryProperties();
		std::vector<VmaBudget> budgets(memoryProps.memoryHeapCount);
		vmaGetBudget(s_Data->Allocator, budgets.data());

		XYZ_CORE_WARN("-----------------------------------");
		for (VmaBudget& b : budgets)
		{
			//XYZ_CORE_WARN("VmaBudget.allocationBytes = {0}", Utils::BytesToString(b.allocationBytes));
			//XYZ_CORE_WARN("VmaBudget.blockBytes = {0}", Utils::BytesToString(b.blockBytes));
			//XYZ_CORE_WARN("VmaBudget.usage = {0}", Utils::BytesToString(b.usage));
			//XYZ_CORE_WARN("VmaBudget.budget = {0}", Utils::BytesToString(b.budget));
		}
		XYZ_CORE_WARN("-----------------------------------");
	}

	GPUMemoryStats VulkanAllocator::GetStats()
	{
		const auto& memoryProps = VulkanContext::GetCurrentDevice()->GetPhysicalDevice()->GetMemoryProperties();
		std::vector<VmaBudget> budgets(memoryProps.memoryHeapCount);
		vmaGetBudget(s_Data->Allocator, budgets.data());

		uint64_t usage = 0;
		uint64_t budget = 0;

		for (const VmaBudget& b : budgets)
		{
			usage += b.usage;
			budget += b.budget;
		}

		return { usage, budget - usage };
	}

	void VulkanAllocator::Init(Ref<VulkanDevice> device)
	{
		s_Data = new VulkanAllocatorData();

		// Initialize VulkanMemoryAllocator
		VmaAllocatorCreateInfo allocatorInfo = {};
		allocatorInfo.vulkanApiVersion = VK_API_VERSION_1_2;
		allocatorInfo.physicalDevice = device->GetPhysicalDevice()->GetVulkanPhysicalDevice();
		allocatorInfo.device = device->GetVulkanDevice();
		allocatorInfo.instance = VulkanContext::GetInstance();

		vmaCreateAllocator(&allocatorInfo, &s_Data->Allocator);
	}

	void VulkanAllocator::Shutdown()
	{
#ifdef XYZ_DEBUG
		for (const auto& [tag, count] : s_AllocationCounter.CountMap)
		{
			XYZ_ASSERT(count == 0, "Allocation with tag not released " + tag);
		}
#endif

		vmaDestroyAllocator(s_Data->Allocator);

		delete s_Data;
		s_Data = nullptr;
	}

	VmaAllocator& VulkanAllocator::GetVMAAllocator()
	{
		return s_Data->Allocator;
	}


	void VulkanAllocator::increaseAllocationCounter()
	{
#ifdef XYZ_DEBUG
		std::scoped_lock lock(s_AllocationCounter.Mutex);
		s_AllocationCounter.CountMap[m_Tag]++;
#endif
	}

	void VulkanAllocator::decreaseAllocationCounter()
	{
#ifdef XYZ_DEBUG
		std::scoped_lock lock(s_AllocationCounter.Mutex);
		s_AllocationCounter.CountMap[m_Tag]--;
#endif
	}

}