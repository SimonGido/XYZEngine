#include "stdafx.h"
#include "Signature.h"

#include "DynamicBitset.h"

namespace XYZ {
	Signature::Signature(int32_t index, DynamicBitset* bitset)
		:
		m_Bitset(bitset),
		m_Index(index)
	{
		XYZ_ASSERT(m_Bitset, "");
		for (uint8_t i = 0; i < m_Bitset->m_BitCount; ++i)
			m_Bitset->m_Bitset[getIndex(*this, i)] = false;
	}

	Signature::Signature(const Signature& other)
		:
		m_Bitset(other.m_Bitset),
		m_Index(other.m_Index)
	{
	}

	Signature::~Signature()
	{
		Reset();
	}

	void Signature::Set(uint8_t bitIndex, bool val)
	{
		size_t index = getIndex(*this, bitIndex);
		m_Bitset->m_Bitset[index] = val;
	}

	void Signature::Reset()
	{
		for (uint8_t i = 0; i < m_Bitset->m_BitCount; ++i)
			m_Bitset->m_Bitset[getIndex(*this, i)] = false;
	}

	bool Signature::operator==(const Signature& other) const
	{
		for (uint8_t i = 0; i < m_Bitset->m_BitCount; ++i)
		{
			if (m_Bitset->m_Bitset[getIndex(*this, i)] != other[i])
				return false;
		}
		return true;
	}
	bool Signature::operator!=(const Signature& other) const
	{
		for (uint8_t i = 0; i < m_Bitset->m_BitCount; ++i)
		{
			if (m_Bitset->m_Bitset[getIndex(*this, i)] != other[i])
				return true;
		}
		return false;
	}
	Signature& Signature::operator=(const Signature& other)
	{
		m_Bitset = other.m_Bitset;
		m_Index = other.m_Index;
		return *this;
	}
	Signature& Signature::operator&=(const Signature& other)
	{
		for (uint8_t i = 0; i < m_Bitset->m_BitCount; ++i)
		{
			size_t index = getIndex(*this, i);
			m_Bitset->m_Bitset[index] = (m_Bitset->m_Bitset[index] && other[i]);
		}
		return *this;
	}
	Signature& Signature::operator|=(const Signature& other)
	{
		for (uint8_t i = 0; i < m_Bitset->m_BitCount; ++i)
		{
			size_t index = getIndex(*this, i);
			m_Bitset->m_Bitset[index] = (m_Bitset->m_Bitset[index] || other[i]);
		}
		return *this;
	}
	Signature Signature::operator&(const Signature& other) const
	{
		Signature result(*this);
		for (uint8_t i = 0; i < m_Bitset->m_BitCount; ++i)
		{
			size_t index = getIndex(*this, i);
			result.Set(i, (m_Bitset->m_Bitset[index] && other[i]));			
		}
		return result;
	}
	Signature Signature::operator|(const Signature& other) const
	{
		Signature result(*this);
		for (uint8_t i = 0; i < m_Bitset->m_BitCount; ++i)
		{
			size_t index = getIndex(*this, i);
			result.Set(i, (m_Bitset->m_Bitset[index] || other[i]));			
		}
		return result;
	}
	bool Signature::operator[](uint8_t bitIndex) const
	{
		return m_Bitset->m_Bitset[getIndex(*this, bitIndex)];
	}
	std::vector<bool>::iterator Signature::begin()
	{
		return m_Bitset->m_Bitset.begin() + getIndex(*this, 0);
	}
	std::vector<bool>::iterator Signature::end()
	{
		return m_Bitset->m_Bitset.begin() + getIndex(*this, m_Bitset->m_BitCount);
	}
	std::vector<bool>::const_iterator Signature::begin() const
	{
		return m_Bitset->m_Bitset.begin() + getIndex(*this, 0);
	}
	std::vector<bool>::const_iterator Signature::end() const
	{
		return m_Bitset->m_Bitset.begin() + getIndex(*this, m_Bitset->m_BitCount);
	}
	uint16_t Signature::Size() const
	{
		return m_Bitset->m_BitCount;
	}
	size_t Signature::getIndex(const Signature& signature, uint8_t bitIndex)
	{
		return (size_t)signature.m_Index * (size_t)signature.m_Bitset->m_BitCount + (size_t)bitIndex;
	}
}