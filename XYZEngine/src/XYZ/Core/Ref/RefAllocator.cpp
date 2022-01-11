#include "stdafx.h"
#include "RefAllocator.h"

#include "XYZ/Utils/DataStructures/MemoryPool.h"

namespace XYZ {

	void* RefAllocator::Allocate(uint32_t size)
	{
		return new std::byte[size];
	}

	void RefAllocator::Deallocate(const void* handle)
	{
		const std::byte* ptr = static_cast<const std::byte*>(handle);
		delete[]ptr;
	}
	
}