#include "stdafx.h"
#include "Ref.h"

#include "XYZ/Utils/DataStructures/MemoryPool.h"

namespace XYZ {

	MemoryPool<1024 * 1024, true>* RefAllocator::s_Pool = nullptr;
	bool						   RefAllocator::s_Initialized = false;

	RefCollector::Container		   RefCollector::s_ReleasedInstances[2];
	RefCollector::Container*	   RefCollector::s_Collecting = &RefCollector::s_ReleasedInstances[0];
	RefCollector::Container*	   RefCollector::s_Releasing  = &RefCollector::s_ReleasedInstances[1];


	void RefAllocator::Init(MemoryPool<1024 * 1024, true>* pool)
	{
		s_Pool = pool;
		s_Initialized = true;
	}
	
	void* RefAllocator::allocate(uint32_t size)
	{
		return s_Pool->AllocateRaw(size);
	}
	void RefAllocator::deallocate(const void* handle)
	{
		s_Pool->DeallocateRaw(handle);
	}

	void RefCollector::AddInstance(const RefCount* ref)
	{
		s_Collecting->push_back(ref);
	}
	void RefCollector::DeleteInstances()
	{
		if (RefAllocator::Initialized())
		{
			for (auto instance : *s_Releasing)
			{
				instance->~RefCount();
				RefAllocator::deallocate(instance);
			}
		}
		else
		{
			for (auto instance : *s_Releasing)
				delete instance;
		}
		s_Releasing->clear();
		std::swap(s_Collecting, s_Releasing);
	}
	void RefCollector::DeleteAll()
	{
		while (!s_Collecting->empty() || !s_Releasing->empty())
			DeleteInstances();
	}
}