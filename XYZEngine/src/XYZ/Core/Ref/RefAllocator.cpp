#include "stdafx.h"
#include "RefAllocator.h"

#include "XYZ/Utils/DataStructures/MemoryPool.h"

namespace XYZ {

	MemoryPool* RefAllocator::s_Pool = nullptr;
	bool		RefAllocator::s_Initialized = false;

	void RefAllocator::Init(MemoryPool* pool)
	{
		s_Pool = pool;
		s_Initialized = true;
	}

	void* RefAllocator::allocate(uint32_t size)
	{
		return s_Pool->Allocate(size);
	}
	void RefAllocator::deallocate(const void* handle)
	{
		s_Pool->Deallocate(handle);
	}

}