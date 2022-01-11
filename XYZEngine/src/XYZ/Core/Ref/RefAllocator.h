#pragma once
#include <stdint.h>

namespace XYZ {
	
	class RefAllocator
	{
	public:
		void* Allocate(uint32_t size);
		void  Deallocate(const void* handle);
	};


	class MemoryPool;
	class RefPoolAllocator
	{
	public:
		void* Allocate(uint32_t size, const char* debugName = "");
		void  Deallocate(const void* handle);
		
	};
}