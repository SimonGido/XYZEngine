#include "stdafx.h"
#include "Pool.h"

namespace XYZ {
	
	Pool::Pool(size_t size)
		: m_Size(size)
	{
		m_Data = new uint8_t[m_Size];
	}
	Pool::Pool(Pool&& other) noexcept
		:
		m_Size(other.m_Size),
		m_Data(other.m_Data)
	{
		other.m_Data = nullptr;
		other.m_Size = 0;
	}
	Pool::~Pool()
	{
		delete[]m_Data;
	}

	Pool& Pool::operator=(Pool&& other) noexcept
	{
		if (m_Data)
			delete[]m_Data;
		m_Data = other.m_Data;
		m_Size = other.m_Size;
		other.m_Data = nullptr;
		other.m_Size = 0;
		return *this;
	}
}