#pragma once
#include <stdint.h>

namespace XYZ {

	class RefAllocator
	{
	public:
		static void* Allocate(uint32_t size, const char* debugName = "");
		static void  Deallocate(const void* handle);
	};


	class MemoryPool;
	class RefPoolAllocator
	{
	public:
		static void  Init(uint32_t blockSize);
	
		static void* Allocate(uint32_t size, const char* debugName = "");
		static void  Deallocate(const void* handle);

	private:
		static MemoryPool s_Pool;
	};
}