#include "stdafx.h"
#include "CustomBuffer.h"


namespace XYZ {
	CustomBuffer::CustomBuffer()
		:
		m_Data(nullptr),
		m_ElementSize(0),
		m_Count(0)
	{
	}
	CustomBuffer::CustomBuffer(size_t elementSize, size_t size)
		:
		m_ElementSize(elementSize),
		m_Count(size)
	{
		Resize(size);
	}
	CustomBuffer::CustomBuffer(const CustomBuffer& other)
		:
		m_ElementSize(other.m_ElementSize),
		m_Count(other.m_Count)
	{
		Resize(m_Count);
		memcpy(m_Data, other.m_Data, m_Count * m_ElementSize);
	}
	CustomBuffer::CustomBuffer(CustomBuffer&& other) noexcept
		:
		m_ElementSize(other.m_ElementSize),
		m_Count(other.m_Count)
	{
		m_Data = other.m_Data;
		other.m_Data = nullptr;
	}
	CustomBuffer::~CustomBuffer()
	{
		Clear();
	}
	CustomBuffer& CustomBuffer::operator=(const CustomBuffer& other)
	{
		m_ElementSize = other.m_ElementSize;
		m_Count = other.m_Count;
		Clear();
		Resize(m_Count);
		memcpy(m_Data, other.m_Data, m_Count * m_ElementSize);
		return *this;
	}
	CustomBuffer& CustomBuffer::operator=(CustomBuffer&& other) noexcept
	{
		m_ElementSize = other.m_ElementSize;
		m_Count = other.m_Count;
		m_Data = other.m_Data;
		other.m_Data = nullptr;
		return *this;
	}
	void CustomBuffer::Resize(size_t count)
	{
		XYZ_ASSERT(m_ElementSize != 0, "Element size is 0");
		size_t newSize = count * m_ElementSize;
		std::byte* temp = new std::byte[newSize];
		if (m_Data)
		{
			size_t copySize = std::min(m_Count, newSize);
			memcpy(temp, m_Data, copySize);
			delete[]m_Data;
		}
		m_Data = temp;
		m_Count = count;
	}
	void CustomBuffer::SetElementSize(size_t size)
	{
		m_ElementSize = size;
	}
	void CustomBuffer::Clear()
	{
		if (m_Data)
		{
			delete[]m_Data;
			m_Data = nullptr;
		}
	}
}