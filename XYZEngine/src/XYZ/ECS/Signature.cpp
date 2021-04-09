#include "stdafx.h"
#include "Signature.h"



namespace XYZ {
	Signature::Signature(uint8_t bitCount,int32_t index, std::vector<bool>& bitset)
		:
		m_Bitset(bitset),
		m_Index(index),
		m_BitCount(bitCount)
	{
		for (uint8_t i = 0; i < m_BitCount; ++i)
			m_Bitset[getIndex(*this, i)] = false;
	}

	Signature::Signature(const Signature& other)
		:
		m_Bitset(other.m_Bitset),
		m_Index(other.m_Index),
		m_BitCount(other.m_BitCount)
	{
	}

	Signature::~Signature()
	{
		Reset();
	}

	void Signature::Set(uint8_t bitIndex, bool val)
	{
		size_t index = getIndex(*this, bitIndex);
		m_Bitset[index] = val;
	}

	void Signature::Reset()
	{
		for (uint8_t i = 0; i < m_BitCount; ++i)
			m_Bitset[getIndex(*this, i)] = false;
	}

	bool Signature::operator==(const Signature& other) const
	{
		for (uint8_t i = 0; i < m_BitCount; ++i)
		{
			if (m_Bitset[getIndex(*this, i)] != other[i])
				return false;
		}
		return true;
	}
	bool Signature::operator!=(const Signature& other) const
	{
		for (uint8_t i = 0; i < m_BitCount; ++i)
		{
			if (m_Bitset[getIndex(*this, i)] != other[i])
				return true;
		}
		return false;
	}
	Signature& Signature::operator=(const Signature& other)
	{
		m_Bitset = other.m_Bitset;
		m_Index = other.m_Index;
		m_BitCount = other.m_BitCount;
		return *this;
	}
	Signature& Signature::operator&=(const Signature& other)
	{
		for (uint8_t i = 0; i < m_BitCount; ++i)
		{
			size_t index = getIndex(*this, i);
			m_Bitset[index] = (m_Bitset[index] && other[i]);
		}
		return *this;
	}
	Signature& Signature::operator|=(const Signature& other)
	{
		for (uint8_t i = 0; i < m_BitCount; ++i)
		{
			size_t index = getIndex(*this, i);
			m_Bitset[index] = (m_Bitset[index] || other[i]);
		}
		return *this;
	}
	Signature Signature::operator&(const Signature& other) const
	{
		Signature result(*this);
		for (uint8_t i = 0; i < m_BitCount; ++i)
		{
			size_t index = getIndex(*this, i);
			result.Set(i, (m_Bitset[index] && other[i]));			
		}
		return result;
	}
	Signature Signature::operator|(const Signature& other) const
	{
		Signature result(*this);
		for (uint8_t i = 0; i < m_BitCount; ++i)
		{
			size_t index = getIndex(*this, i);
			result.Set(i, (m_Bitset[index] || other[i]));			
		}
		return result;
	}
	bool Signature::operator[](uint8_t bitIndex) const
	{
		return m_Bitset[getIndex(*this, bitIndex)];
	}
	size_t Signature::getIndex(const Signature& signature, uint8_t bitIndex)
	{
		return (size_t)signature.m_Index * (size_t)signature.m_BitCount + (size_t)bitIndex;
	}
}