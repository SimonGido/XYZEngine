#include "stdafx.h"
#include "SparseArray.h"

namespace XYZ {
	OptionalIndex::OptionalIndex(uint32_t index)
		: m_Value(index)
	{}

	OptionalIndex& OptionalIndex::operator=(uint32_t index)
	{
		m_Value = index;
		return *this;
	}

}
