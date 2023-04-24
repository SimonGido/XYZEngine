#pragma once
#include "Buffer.h"

#include <thread>
#include <shared_mutex>
#include <queue>

namespace XYZ {
	

	class StorageBufferAllocator;
	class XYZ_API StorageBufferAllocation
	{
	public:
		StorageBufferAllocation();
		~StorageBufferAllocation();

		StorageBufferAllocation(const StorageBufferAllocation& other);
		StorageBufferAllocation(StorageBufferAllocation&& other) noexcept;

		StorageBufferAllocation& operator=(const StorageBufferAllocation& other);
		StorageBufferAllocation& operator=(StorageBufferAllocation&& other) noexcept;

		StorageBufferAllocation CreateSubAllocation(uint32_t offset, uint32_t size);

		inline uint32_t   GetSize()			const { return m_Size; }
		inline uint32_t   GetOffset()		const { return m_Offset; }
		inline uint32_t   GetBinding()		const { return m_StorageBufferBinding; }
		inline uint32_t   GetSet()			const { return m_StorageBufferSet; }
		inline bool		  Valid()			const { return m_Valid; }
	private:
		StorageBufferAllocation(
			const Ref<StorageBufferAllocator>& allocator, 
			uint32_t size, 
			uint32_t offset,
			uint32_t binding,
			uint32_t set,
			int32_t id,
			bool isSuballocation
		);

		void returnAllocation();
		
	private:
		Ref<StorageBufferAllocator> m_Allocator;
		
		uint32_t m_Size;
		uint32_t m_Offset;
		uint32_t m_StorageBufferBinding;
		uint32_t m_StorageBufferSet;
		uint32_t m_ID;
		bool	 m_Valid;
		bool	 m_IsSuballocation;

		friend StorageBufferAllocator;
	};


	class XYZ_API StorageBufferAllocator : public RefCount
	{
	public:
		StorageBufferAllocator(uint32_t size, uint32_t binding, uint32_t set);
		~StorageBufferAllocator();

		bool Allocate(uint32_t size, StorageBufferAllocation& allocation);

		uint32_t GetAllocatedSize() const;
		uint32_t GetBinding()		const { return m_Binding; };
		uint32_t GetSet()			const { return m_Set; }
		uint32_t GetSize()			const { return m_Size; }
	private:
		bool returnAllocation(uint32_t size, uint32_t offset, uint32_t id);
		bool allocateFromFree(uint32_t size, uint32_t& offset);

		bool reallocationRequired(uint32_t size, const StorageBufferAllocation& allocation) const;

		StorageBufferAllocation createNewAllocation(uint32_t size);
		void updateAllocation(uint32_t size, StorageBufferAllocation& allocation);

		uint32_t nextAllocationID();
	private:
		struct Allocation
		{
			uint32_t Size;
			uint32_t Offset;
		};

		uint32_t m_Binding;
		uint32_t m_Set;
		uint32_t m_Size;


		std::vector<Allocation>	m_FreeAllocations;
		std::vector<uint32_t>	m_AllocationRefCounter;
		std::queue<uint32_t>    m_FreeAllocationIDs;

		std::uint32_t			m_Next;
		uint32_t				m_AllocatedSize;
		uint32_t				m_UnusedSpace;
		bool					m_SortRequired;


		friend StorageBufferAllocation;
	};
}