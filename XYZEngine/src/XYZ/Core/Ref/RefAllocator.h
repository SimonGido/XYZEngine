#pragma once
#include <stdint.h>

namespace XYZ {
	template<uint32_t BlockSize, bool StoreSize>
	class MemoryPool;

	template <typename T>
	class Ref;

	class RefAllocator
	{
	public:
		static void  Init(MemoryPool<1024 * 1024, true>* pool);

		static bool  Initialized() { return s_Initialized; }
		template <typename T>
		static void Deallocate(const T* instance)
		{
			instance->~T();
			deallocate(instance);
		}

	private:
		static void* allocate(uint32_t size);
		static void  deallocate(const void* handle);

		static MemoryPool<1024 * 1024, true>* s_Pool;
		static bool							  s_Initialized;

		template <typename T>
		friend class Ref;
	};
}