#include "stdafx.h"
#include "RefAllocator.h"

#include "XYZ/Utils/DataStructures/MemoryPool.h"

namespace XYZ {

	void* RefAllocator::Allocate(uint32_t size, const char* debugName)
	{
		return new std::byte[size];
	}

	void RefAllocator::Deallocate(const void* handle)
	{
		delete[]handle;
	}

	MemoryPool  RefPoolAllocator::s_Pool = MemoryPool(1024 * 1024 * 10);

	void RefPoolAllocator::Init(uint32_t blockSize)
	{
		s_Pool = MemoryPool(blockSize);
	}
	void* RefPoolAllocator::Allocate(uint32_t size, const char* debugName)
	{
		return s_Pool.Allocate(size, debugName);
	}
	void RefPoolAllocator::Deallocate(const void* handle)
	{
		s_Pool.Deallocate(handle);
	}
}