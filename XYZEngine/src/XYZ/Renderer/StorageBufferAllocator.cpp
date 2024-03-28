#include "stdafx.h"	
#include "StorageBufferSet.h"
#include "StorageBufferAllocator.h"

#include "XYZ/Debug/Profiler.h"

namespace XYZ {

	StorageBufferAllocation::StorageBufferAllocation()
		:
		m_Allocator(nullptr),
		m_Size(0),
		m_Offset(0),
		m_ID(std::numeric_limits<uint32_t>::max()),
		m_StorageBufferBinding(0),
		m_StorageBufferSet(0),
		m_Valid(false),
		m_IsSuballocation(false)
	{
	}

	StorageBufferAllocation::StorageBufferAllocation(
		const Ref<StorageBufferAllocator>& allocator,
		uint32_t size,
		uint32_t offset,
		uint32_t binding,
		uint32_t set,
		int32_t id,
		bool isSuballocation
	)
		:
		m_Allocator(allocator),
		m_Size(size),
		m_Offset(offset),
		m_StorageBufferBinding(binding),
		m_StorageBufferSet(set),
		m_ID(id),
		m_Valid(true),
		m_IsSuballocation(isSuballocation)
	{
		if (m_Valid && !m_IsSuballocation)
			m_Allocator->m_AllocationRefCounter[m_ID]++;
	}
	
	StorageBufferAllocation::~StorageBufferAllocation()
	{
		returnAllocation();
	}

	StorageBufferAllocation::StorageBufferAllocation(const StorageBufferAllocation& other)
		:
		m_Allocator(other.m_Allocator),
		m_Size(other.m_Size),
		m_Offset(other.m_Offset),
		m_StorageBufferBinding(other.m_StorageBufferBinding),
		m_StorageBufferSet(other.m_StorageBufferSet),
		m_ID(other.m_ID),
		m_Valid(other.m_Valid),
		m_IsSuballocation(other.m_IsSuballocation)
	{
		if (m_Valid && !m_IsSuballocation)
			m_Allocator->m_AllocationRefCounter[m_ID]++;
	}

	StorageBufferAllocation::StorageBufferAllocation(StorageBufferAllocation&& other) noexcept
		:
		m_Allocator(other.m_Allocator),
		m_Size(other.m_Size),
		m_Offset(other.m_Offset),
		m_StorageBufferBinding(other.m_StorageBufferBinding),
		m_StorageBufferSet(other.m_StorageBufferSet),
		m_ID(other.m_ID),
		m_Valid(other.m_Valid),
		m_IsSuballocation(other.m_IsSuballocation)
	{
		if (m_Valid && !m_IsSuballocation)
			m_Allocator->m_AllocationRefCounter[m_ID]++;
	}


	StorageBufferAllocation& StorageBufferAllocation::operator=(const StorageBufferAllocation& other)
	{
		returnAllocation();
		m_Allocator = other.m_Allocator;
		m_Size = other.m_Size;
		m_Offset = other.m_Offset;
		m_StorageBufferBinding = other.m_StorageBufferBinding;
		m_StorageBufferSet = other.m_StorageBufferSet;
		m_ID = other.m_ID;
		m_Valid = other.m_Valid;
		m_IsSuballocation = other.m_IsSuballocation;
		if (m_Valid && !m_IsSuballocation)
			m_Allocator->m_AllocationRefCounter[m_ID]++;
		return *this;
	}

	StorageBufferAllocation& StorageBufferAllocation::operator=(StorageBufferAllocation&& other) noexcept
	{
		returnAllocation();
		m_Allocator = other.m_Allocator;
		m_Size = other.m_Size;
		m_Offset = other.m_Offset;
		m_StorageBufferBinding = other.m_StorageBufferBinding;
		m_StorageBufferSet = other.m_StorageBufferSet;
		m_ID = other.m_ID;
		m_Valid = other.m_Valid;
		m_IsSuballocation = other.m_IsSuballocation;
		
		if (m_Valid && !m_IsSuballocation)
			m_Allocator->m_AllocationRefCounter[m_ID]++;
		
		return *this;
	}


	StorageBufferAllocation StorageBufferAllocation::CreateSubAllocation(uint32_t offset, uint32_t size)
	{
		XYZ_ASSERT(offset + size <= m_Size, "");

		StorageBufferAllocation subAllocation(
			m_Allocator, size, m_Offset + offset, m_StorageBufferBinding, m_StorageBufferSet, m_ID, true
		);

		return subAllocation;
	}

	void StorageBufferAllocation::returnAllocation()
	{
		if (m_Valid && !m_IsSuballocation)
		{
			m_Allocator->returnAllocation(m_Size, m_Offset, m_ID);
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
		m_UnusedSpace(0),
		m_SortRequired(false)
	{
	}

	StorageBufferAllocator::~StorageBufferAllocator()
	{
	}

	
	uint32_t StorageBufferAllocator::Allocate(uint32_t size, StorageBufferAllocation& allocation)
	{
		XYZ_PROFILE_FUNC("StorageBufferAllocator::Allocate");
		uint32_t flags = 0;
		// Allocation is not valid, create new
		if (!allocation.m_Valid)
		{
			allocation = createNewAllocation(size);
			m_AllocatedSize += size;
			m_UnusedSpace = m_Next - m_AllocatedSize;
			flags |= Reallocated;
		}
		else if (reallocationRequired(size, allocation))
		{		
			if (extendAllocation(size, allocation))
			{
				flags |= Extended;
			}
			else
			{
				updateAllocation(size, allocation);
				flags |= Reallocated;
			}
			m_AllocatedSize += size;
			m_UnusedSpace = m_Next - m_AllocatedSize;
		}
			
		return flags;
	}


	uint32_t StorageBufferAllocator::GetAllocatedSize() const
	{
		return m_AllocatedSize;
	}

	bool StorageBufferAllocator::returnAllocation(uint32_t size, uint32_t offset, uint32_t id)
	{
		XYZ_ASSERT(m_AllocationRefCounter.size() > static_cast<size_t>(id), "");
		XYZ_ASSERT(m_AllocationRefCounter[id] != 0, "");

		m_AllocationRefCounter[id]--;
		if (m_AllocationRefCounter[id] == 0)
		{
			m_FreeAllocations.push_back({ size, offset });
			m_FreeAllocationIDs.push(id);
			m_AllocatedSize -= size;
			m_UnusedSpace = m_Next - m_AllocatedSize;
			m_SortRequired = true;
			return true;
		}
		return false;
	}

	bool StorageBufferAllocator::allocateFromFree(uint32_t size, uint32_t& offset)
	{
		XYZ_PROFILE_FUNC("StorageBufferAllocator::allocateFromFree");
		
		if (m_SortRequired)
		{
			std::sort(m_FreeAllocations.begin(), m_FreeAllocations.end(), [](const Allocation& a, const Allocation& b) {
				return a.Offset < b.Offset;
				});
			m_SortRequired = false;
		}

		// Merge free allocations
		for (int64_t i = m_FreeAllocations.size() - 1; i >= 1; --i)
		{
			auto& last = m_FreeAllocations[i];
			auto& prev = m_FreeAllocations[i - 1];

			
			if (last.Offset == prev.Offset + prev.Size)
			{
				// Merge last allocation with previous allocation if possible
				prev.Size += last.Size;
				m_FreeAllocations.erase(m_FreeAllocations.begin() + i);
			}			
		}

		// Try to find suitable free allocation
		for (int64_t i = m_FreeAllocations.size() - 1; i >= 0; --i)
		{
			auto& last = m_FreeAllocations[i];
			if (last.Size > size)
			{
				// Cut required size from free allocation
				last.Size -= size;
				offset = last.Offset + last.Size;
				return true;
			}
			else if (last.Size == size)
			{
				// Take whole free allocation
				offset = last.Offset;
				m_FreeAllocations.erase(m_FreeAllocations.begin() + i);
				return true;
			}
		}
		return false;
	}

	bool StorageBufferAllocator::reallocationRequired(uint32_t size, const StorageBufferAllocation& allocation) const
	{
		return allocation.m_Size < size			 // Size is not sufficient
			|| allocation.GetBinding() != m_Binding // Allocation is not owned by this allocator
			|| allocation.GetSet() != m_Set;		 // Allocation is not owned by this allocator
	}

	StorageBufferAllocation StorageBufferAllocator::createNewAllocation(uint32_t size)
	{
		uint32_t offset = m_Next;
		if (!allocateFromFree(size, offset))
		{
			XYZ_ASSERT(m_Next + size < m_Size, "");
			m_Next += size;
		}
		return StorageBufferAllocation(this, size, offset, m_Binding, m_Set, nextAllocationID(), false);
	}
	void StorageBufferAllocator::updateAllocation(uint32_t size, StorageBufferAllocation& allocation)
	{
		uint32_t offset = m_Next;
		
		if (!allocateFromFree(size, offset))
		{
			XYZ_ASSERT(m_Next + size < m_Size, "");
			m_Next += size;
		}
		allocation.returnAllocation();
		allocation.m_Allocator = this;
		allocation.m_Size = size;
		allocation.m_Offset = offset;
		allocation.m_StorageBufferBinding = m_Binding;
		allocation.m_StorageBufferSet = m_Set;
		allocation.m_Valid = true;
		allocation.m_IsSuballocation = false;
		allocation.m_ID = nextAllocationID();
		m_AllocationRefCounter[allocation.m_ID]++;
		m_Next += size;
	}

	bool StorageBufferAllocator::extendAllocation(uint32_t size, StorageBufferAllocation& allocation)
	{
		if (allocation.m_Offset + allocation.m_Size == m_Next) // It is possible to just extend allocation
		{
			m_AllocatedSize -= allocation.m_Size;
			allocation.m_Size = size; // Update size
			m_Next = allocation.m_Offset + allocation.m_Size; // Calculate new m_Next
			return true;
		}
		return false;
	}

	uint32_t StorageBufferAllocator::nextAllocationID()
	{
		uint32_t result;
		if (!m_FreeAllocationIDs.empty())
		{
			result = m_FreeAllocationIDs.front();
			m_FreeAllocationIDs.pop();
		}
		else
		{
			result = static_cast<uint32_t>(m_AllocationRefCounter.size());
			m_AllocationRefCounter.push_back(0);
		}
		return result;
	}

}