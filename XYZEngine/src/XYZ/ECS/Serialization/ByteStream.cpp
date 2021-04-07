#include "stdafx.h"
#include "ByteStream.h"

namespace XYZ {

	ByteStream::ByteStream()
		:
		m_Data(nullptr),
		m_Size(0),
		m_Capacity(0)
	{
	}
	ByteStream::ByteStream(const ByteStream& other)
		:
		m_Size(other.m_Size),
		m_Capacity(other.m_Capacity)
	{
		m_Data = new uint8_t[other.m_Capacity];
		memcpy(m_Data, other.m_Data, other.m_Size);
	}
	ByteStream::ByteStream(ByteStream&& other) noexcept
		:
		m_Size(other.m_Size),
		m_Capacity(other.m_Capacity)
	{
		m_Data = other.m_Data;
		other.m_Data = nullptr;
		other.m_Capacity = 0;
		other.m_Size = 0;
	}
	ByteStream::~ByteStream()
	{
		if (m_Data)
		{
			delete[]m_Data;
		}
	}
	void ByteStream::handleMemorySize(size_t sizeReq)
	{
		if (m_Size + sizeReq > m_Capacity)
		{
			size_t newCapacity = (m_Capacity + 1) * sc_CapacityMultiplier;
			if (newCapacity - m_Capacity < sizeReq)
				newCapacity += sizeReq;
			m_Capacity = newCapacity;
			uint8_t* tmp = new uint8_t[m_Capacity];
			if (m_Data)
			{
				memcpy(tmp, m_Data, m_Size);
				delete[]m_Data;
			}
			m_Data = tmp;
		}
	}
}