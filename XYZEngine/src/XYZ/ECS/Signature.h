#pragma once

namespace XYZ {

	class DynamicBitset;
	class Signature
	{
	public:
		Signature(int32_t index, DynamicBitset* bitset);
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

		std::vector<bool>::iterator begin();
		std::vector<bool>::iterator end();
		std::vector<bool>::const_iterator begin() const;
		std::vector<bool>::const_iterator end()   const;

		int32_t GetIndex() const { return m_Index; }
	private:
		static size_t getIndex(const Signature& signature, uint8_t bitIndex);

	private:
		DynamicBitset* m_Bitset;
		int32_t m_Index;

		friend class DynamicBitset;
	};

}