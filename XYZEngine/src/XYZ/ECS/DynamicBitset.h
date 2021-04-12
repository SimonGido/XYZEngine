#pragma once
#include "XYZ/Utils/DataStructures/FreeList.h"
#include "Signature.h"

namespace XYZ {

	class DynamicBitset
	{
	public:
		DynamicBitset() = default;
		DynamicBitset(const DynamicBitset& other);
		DynamicBitset(DynamicBitset&& other) noexcept;

		DynamicBitset& operator =(DynamicBitset&& other) noexcept;

		int32_t CreateSignature();
		void DestroySignature(int32_t index);

		Signature& GetSignature(int32_t index);
		const Signature& GetSignature(int32_t index) const;

		void SetNumberBits(uint8_t count);
		void Clear();

		Signature& operator[](int32_t index);		
		const Signature& operator[](int32_t index) const;
		
		size_t GetNumberOfSignatures() const { return m_Signatures.Range(); }
	private:
		FreeList<Signature> m_Signatures;
		// Bitset must be under signatures or it will be destroyed earlier -> results in crash
		std::vector<bool> m_Bitset;

		uint8_t m_BitCount = 0;

		friend class Signature;
	};

}