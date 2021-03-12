#include "stdafx.h"
#include "ComponentAllocator.h"



namespace XYZ {
	ComponentAllocator::ComponentAllocator()
		:
		m_ID(INVALID_COMPONENT),
		m_ElementSize(0)
	{}
	
	void ComponentAllocator::Init(uint8_t id, size_t elementSize)
	{
		m_ID = id;
		m_ElementSize = elementSize;
		m_Initialized = true;
	}
}