#pragma once

namespace XYZ {

	class DynamicBitset;
	class Signature
	{
	public:
		Signature(int32_t index, DynamicBitset* bitset);
		Signature(const Signature& other);
		~Signature();

		void Set(uint16_t bitIndex, bool val = true);
		void Reset();
		bool And(const Signature& other) const;

		Signature& operator =(const Signature& other);
		Signature& operator &=(const Signature& other);
		Signature& operator |=(const Signature& other);

		bool operator ==(const Signature& other) const;
		bool operator !=(const Signature& other) const;

		bool operator[](uint16_t bitIndex) const;

		std::vector<bool>::iterator begin();
		std::vector<bool>::iterator end();
		std::vector<bool>::const_iterator begin() const;
		std::vector<bool>::const_iterator end()   const;

		uint16_t Size() const;
	private:
		static size_t getIndex(const Signature& signature, uint16_t bitIndex);

	private:
		DynamicBitset* m_Bitset;
		int32_t m_Index;

		friend class DynamicBitset;
	};

}