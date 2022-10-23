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
		returnAllocation();
	}

	void StorageBufferAllocation::returnAllocation()
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
		m_UnusedSpace(0),
		m_SortRequired(false)
	{
	}

	StorageBufferAllocator::~StorageBufferAllocator()
	{
	}

	
	bool StorageBufferAllocator::Allocate(uint32_t size, Ref<StorageBufferAllocation>& allocation)
	{
		XYZ_PROFILE_FUNC("StorageBufferAllocator::Allocate");
		bool allocatedNew = false;
		// Allocation is not valid, create new
		if (!allocation.Raw())
		{
			allocation = createNewAllocation(size);
			allocatedNew = true;
		}
		else if (reallocationRequired(size, allocation))
		{		
			updateAllocation(size, allocation);
			allocatedNew = true;
		}
			
		m_AllocatedSize += size;
		m_UnusedSpace = m_Next - m_AllocatedSize;
		return allocatedNew;
	}

	uint32_t StorageBufferAllocator::GetAllocatedSize() const
	{
		return m_AllocatedSize;
	}

	void StorageBufferAllocator::returnAllocation(uint32_t size, uint32_t offset)
	{
		m_FreeAllocations.push_back({ size, offset });
		m_AllocatedSize -= size;
		m_UnusedSpace = m_Next - m_AllocatedSize;
		m_SortRequired = true;
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

	bool StorageBufferAllocator::reallocationRequired(uint32_t size, Ref<StorageBufferAllocation>& allocation)
	{
		return allocation->m_Size < size			 // Size is not sufficient
			|| allocation->GetBinding() != m_Binding // Allocation is not owned by this allocator
			|| allocation->GetSet() != m_Set;		 // Allocation is not owned by this allocator
	}

	Ref<StorageBufferAllocation> StorageBufferAllocator::createNewAllocation(uint32_t size)
	{
		uint32_t offset = m_Next;
		if (!allocateFromFree(size, offset))
		{
			XYZ_ASSERT(m_Next + size < m_Size, "");
			m_Next += size;
		}
		return Ref<StorageBufferAllocation>(new StorageBufferAllocation(this, size, offset, m_Binding, m_Set));
	}
	void StorageBufferAllocator::updateAllocation(uint32_t size, Ref<StorageBufferAllocation>& allocation)
	{
		uint32_t offset = m_Next;
		if (!allocateFromFree(size, offset))
		{
			XYZ_ASSERT(m_Next + size < m_Size, "");
			m_Next += size;
		}
		allocation->returnAllocation();
		allocation->m_Allocator = this;
		allocation->m_Size = size;
		allocation->m_Offset = offset;
		allocation->m_StorageBufferBinding = m_Binding;
		allocation->m_StorageBufferSet = m_Set;
		allocation->m_Valid = true;
		m_Next += size;
	}

}