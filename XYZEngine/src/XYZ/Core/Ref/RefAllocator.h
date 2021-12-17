#pragma once
#include <stdint.h>

namespace XYZ {

	template <typename T>
	class Ref;

	class MemoryPool;

	class RefAllocator
	{
	public:
		static void Init(MemoryPool* pool);

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

		static MemoryPool* s_Pool;
		static bool		  s_Initialized;

		template <typename T>
		friend class Ref;
	};
}