#include "stdafx.h"
#include "Ref.h"

#include "XYZ/Utils/DataStructures/MemoryPool.h"

namespace XYZ {

	MemoryPool<1024 * 1024, true>* RefAllocator::s_Pool = nullptr;
	
	void RefAllocator::Init(MemoryPool<1024 * 1024, true>* pool)
	{
		s_Pool = pool;
	}
	void* RefAllocator::allocate(size_t size)
	{
		return s_Pool->AllocateRaw(size);
	}
	void RefAllocator::deallocate(void* handle)
	{
		s_Pool->DeallocateRaw(handle);
	}
}