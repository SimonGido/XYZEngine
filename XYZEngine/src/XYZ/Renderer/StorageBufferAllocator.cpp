#include "stdafx.h"	
#include "StorageBufferSet.h"
#include "StorageBufferAllocator.h"

#include "XYZ/Debug/Profiler.h"

namespace XYZ {

	StorageBufferAllocation::StorageBufferAllocation(
		const Ref<StorageBufferAllocator>& allocator,
		uint32_t size,
		uint32_t offset,
		uint32_t binding,
		uint32_t set
	)
		:
		m_Allocator(allocator),
		m_Size(size),
		m_Offset(offset),
		m_StorageBufferBinding(binding),
		m_StorageBufferSet(set),
		m_Valid(true)
	{
	}
	
	StorageBufferAllocation::~StorageBufferAllocation()
	{
		ReturnAllocation();
	}

	void StorageBufferAllocation::ReturnAllocation()
	{
		if (m_Valid)
		{
			m_Allocator->returnAllocation(m_Size, m_Offset);
			m_Valid = false;
		}
	}
	StorageBufferAllocator::StorageBufferAllocator(uint32_t size, uint32_t binding, uint32_t set)
		:
		m_Binding(binding),
		m_Set(set),
		m_Size(size),
		m_Next(0),
		m_AllocatedSize(0),
		m_Running(true)
	{
		Ref<StorageBufferAllocator> instance = this;
		m_FreeAllocationsThread = std::make_unique<std::thread>(&StorageBufferAllocator::worker, instance);
	}

	StorageBufferAllocator::~StorageBufferAllocator()
	{
		m_Running = false;
		m_FreeAllocationAvailableCV.notify_one();
		m_FreeAllocationsThread->join();
	}

	
	void StorageBufferAllocator::Allocate(uint32_t size, Ref<StorageBufferAllocation>& allocation)
	{
		XYZ_PROFILE_FUNC("StorageBufferAllocator::Allocate");
		XYZ_ASSERT(m_Next + size < m_Size, "");
		
		uint32_t offset = m_Next;
		if (!tryAllocateFromLastFree(size, offset))
			m_Next += size;

		// Allocation is not valid, create new
		if (!allocation.Raw())
		{
			allocation = Ref<StorageBufferAllocation>(new StorageBufferAllocation(this, size, offset, m_Binding, m_Set));
		}
		else 
		{		
			allocation->m_Allocator				= this;
			allocation->m_Size					= size;
			allocation->m_Offset				= offset;
			allocation->m_StorageBufferBinding	= m_Binding;
			allocation->m_StorageBufferSet		= m_Set;
			allocation->m_Valid					= true;
		}
			
		m_AllocatedSize += size;
	}

	uint32_t StorageBufferAllocator::GetAllocatedSize() const
	{
		return m_AllocatedSize;
	}

	void StorageBufferAllocator::returnAllocation(uint32_t size, uint32_t offset)
	{
		std::unique_lock lock(m_FreeAllocationsMutex);
		if (m_Next == offset + size)
		{
			m_Next -= size;
		}
		else
		{
			m_FreeAllocations.push_back({ size, offset });
			m_FreeAllocationAvailableCV.notify_one();
		}
		m_AllocatedSize -= size;
	}

	void StorageBufferAllocator::worker()
	{
		while (true)
		{		
			std::unique_lock lock(m_FreeAllocationsMutex);
			m_FreeAllocationAvailableCV.wait(lock, [&] { return !m_FreeAllocations.empty() || !m_Running; });
			
			if (!m_Running)
				return;

			mergeFreeAllocations();
		}
	}
	void StorageBufferAllocator::mergeFreeAllocations()
	{
		XYZ_PROFILE_FUNC("StorageBufferAllocator::mergeFreeAllocations");
		// Sort allocations based on offset
		std::sort(m_FreeAllocations.begin(), m_FreeAllocations.end(), [](const Allocation a, const Allocation b) {
			return a.Offset < b.Offset;
			});

		for (size_t i = m_FreeAllocations.size() - 1; i >= 1; --i)
		{
			auto& last = m_FreeAllocations[i];
			auto& prev = m_FreeAllocations[i - 1];

			if (last.Offset == prev.Offset + prev.Size)
			{
				// Try merge last free allocation with previous free allocation to decrease fragmentation
				prev.Size += last.Size;
				m_FreeAllocations.erase(m_FreeAllocations.begin() + i);
			}
		}
		if (!m_FreeAllocations.empty())
		{
			auto last = m_FreeAllocations.back();
			// If last free allocation is dirrectly connected to next, just decrease next and remove free allocation
			if (last.Offset + last.Size == m_Next)
			{
				m_Next -= last.Size;
				m_FreeAllocations.erase(m_FreeAllocations.end() - 1);
			}
		}
	}

	bool StorageBufferAllocator::tryAllocateFromLastFree(uint32_t size, uint32_t& offset)
	{
		std::unique_lock lock(m_FreeAllocationsMutex);
		if (!m_FreeAllocations.empty())
		{
			auto& last = m_FreeAllocations.back();
			if (last.Size > size)
			{
				last.Size -= size;
				uint32_t offset = last.Offset + last.Size;
				return true;
			}
			else if (last.Size == size)
			{
				offset = last.Offset;
				m_FreeAllocations.pop_back();
				return true;
			}
		}
		return false;
	}

}