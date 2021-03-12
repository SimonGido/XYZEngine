#include "stdafx.h"
#include "Pool.h"

namespace XYZ {
	
	Pool::Pool(size_t size)
		: m_Size(size)
	{
		m_Data = new uint8_t[m_Size];
	}
	Pool::~Pool()
	{
		delete[]m_Data;
	}
}