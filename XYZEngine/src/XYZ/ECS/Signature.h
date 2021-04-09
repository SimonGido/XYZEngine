#pragma once

namespace XYZ {

	class Signature
	{
	public:
		Signature(uint8_t bitCount, int32_t index, std::vector<bool>& bitset);
		Signature(const Signature& other);
		~Signature();

		void Set(uint8_t bitIndex, bool val = true);
		void Reset();

		Signature& operator =(const Signature& other);
		Signature& operator &=(const Signature& other);
		Signature& operator |=(const Signature& other);

		bool operator ==(const Signature& other) const;
		bool operator !=(const Signature& other) const;
		Signature operator &(const Signature& other) const;
		Signature operator |(const Signature& other) const;

		bool operator[](uint8_t bitIndex) const;

		std::vector<bool>::iterator begin() { return m_Bitset.begin() + getIndex(*this, 0); }
		std::vector<bool>::iterator end()   { return m_Bitset.begin() + getIndex(*this, m_BitCount); }
		std::vector<bool>::const_iterator begin() const { return m_Bitset.begin() + getIndex(*this, 0); }
		std::vector<bool>::const_iterator end()   const { return m_Bitset.begin() + getIndex(*this, m_BitCount); }

		int32_t GetIndex() const { return m_Index; }
	private:
		static size_t getIndex(const Signature& signature, uint8_t bitIndex);

	private:
		std::vector<bool>& m_Bitset;

		int32_t m_Index;
		uint8_t m_BitCount;	

		friend class DynamicBitset;
	};

}