#include "stdafx.h"
#include "Ref.h"

#include "XYZ/Utils/DataStructures/MemoryPool.h"

namespace XYZ {

	MemoryPool* RefAllocator::s_Pool = nullptr;
	
	void RefAllocator::Init(MemoryPool* pool)
	{
		s_Pool = pool;
	}
	void* RefAllocator::allocate(size_t size)
	{
		return s_Pool->AllocateRaw(size);
	}
	void RefAllocator::deallocate(void* handle, size_t size)
	{
		s_Pool->DeallocateRaw(handle, size);
	}
}