#pragma once
#include "Buffer.h"

#include <thread>
#include <shared_mutex>

namespace XYZ {
	

	class StorageBufferAllocator;
	class StorageBufferAllocation : public RefCount
	{
	public:
		StorageBufferAllocation();
		~StorageBufferAllocation();

		Ref<StorageBufferAllocation> CreateSubAllocation(uint32_t offset, uint32_t size);

		inline uint32_t   GetSize()			const { return m_Size; }
		inline uint32_t   GetOffset()		const { return m_Offset; }
		inline uint32_t   GetBinding()		const { return m_StorageBufferBinding; }
		inline uint32_t   GetSet()			const { return m_StorageBufferSet; }
	private:
		StorageBufferAllocation(
			const Ref<StorageBufferAllocator>& allocator, 
			uint32_t size, 
			uint32_t offset,
			uint32_t binding,
			uint32_t set
		);

		void returnAllocation();
		
	private:
		Ref<StorageBufferAllocator> m_Allocator;
		Ref<StorageBufferAllocation> m_Owner;

		uint32_t m_Size;
		uint32_t m_Offset;
		uint32_t m_StorageBufferBinding;
		uint32_t m_StorageBufferSet;
		bool	 m_Valid;

		friend StorageBufferAllocator;
	};


	class StorageBufferAllocator : public RefCount
	{
	public:
		StorageBufferAllocator(uint32_t size, uint32_t binding, uint32_t set);
		~StorageBufferAllocator();

		bool Allocate(uint32_t size, Ref<StorageBufferAllocation>& allocation);

		uint32_t GetAllocatedSize() const;
		uint32_t GetBinding()		const { return m_Binding; };
		uint32_t GetSet()			const { return m_Set; }
		uint32_t GetSize()			const { return m_Size; }
	private:
		void returnAllocation(uint32_t size, uint32_t offset);
		bool allocateFromFree(uint32_t size, uint32_t& offset);

		bool reallocationRequired(uint32_t size, Ref<StorageBufferAllocation>& allocation);

		Ref<StorageBufferAllocation> createNewAllocation(uint32_t size);
		void updateAllocation(uint32_t size, Ref<StorageBufferAllocation>& allocation);

	private:
		struct Allocation
		{
			uint32_t Size, Offset;
		};

		uint32_t m_Binding;
		uint32_t m_Set;
		uint32_t m_Size;


		std::vector<Allocation>	m_FreeAllocations;
		std::uint32_t			m_Next;
		uint32_t				m_AllocatedSize;
		uint32_t				m_UnusedSpace;
		bool					m_SortRequired;


		friend StorageBufferAllocation;
	};
}