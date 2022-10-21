#pragma once
#include "Buffer.h"

#include <thread>
#include <shared_mutex>

namespace XYZ {
	
	class StorageBufferAllocator;
	class StorageBufferAllocation : public RefCount
	{
	public:
		inline uint32_t GetSize() const { return m_Size; }
		inline uint32_t GetOffset() const { return m_Offset; }
	
	private:
		StorageBufferAllocation(const Ref<StorageBufferAllocator>& allocator, uint32_t size, uint32_t offset);

	private:
		Ref<StorageBufferAllocator> m_Allocator;
		uint32_t m_Size;
		uint32_t m_Offset;


		friend StorageBufferAllocator;
	};


	class StorageBufferAllocator : public RefCount
	{
	public:
		StorageBufferAllocator(uint32_t size);
		~StorageBufferAllocator();

		void Allocate(uint32_t size, Ref<StorageBufferAllocation>& allocation);
		bool TryAllocate(uint32_t size, Ref<StorageBufferAllocation>& allocation);

		uint32_t GetAllocatedSize() const;
	private:
		void returnAllocation(uint32_t size, uint32_t offset);
		void worker();
		void mergeFreeAllocations();

	private:
		struct Allocation
		{
			uint32_t Size, Offset;
		};

		uint32_t m_Size;

		std::unique_ptr<std::thread> m_FreeAllocationsThread;
		std::condition_variable		 m_FreeAllocationAvailableCV;

		std::vector<Allocation>		 m_FreeAllocations;
		std::mutex					 m_FreeAllocationsMutex;
		mutable std::shared_mutex	 m_NextMutex;
		uint32_t					 m_Next;
		uint32_t					 m_AllocatedSize;

		bool m_Running;
	};
}