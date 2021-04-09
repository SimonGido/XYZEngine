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

		void CreateSignature();
		void DestroySignature(int32_t index);

		Signature& GetSignature(int32_t index);
		const Signature& GetSignature(int32_t index) const;

		void SetNumberBits(uint8_t count);
	private:
		FreeList<Signature> m_Signatures;
		// Bitset must be under signatures or it will be destroyed earlier -> results in crash
		std::vector<bool> m_Bitset;

		static uint8_t s_BitCount;
	};

}