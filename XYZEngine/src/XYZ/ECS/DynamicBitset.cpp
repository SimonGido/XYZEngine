#include "stdafx.h"
#include "DynamicBitset.h"


namespace XYZ {

    uint8_t DynamicBitset::s_BitCount = 0;

    DynamicBitset::DynamicBitset(const DynamicBitset& other)
        :
        m_Signatures(other.m_Signatures),
        m_Bitset(other.m_Bitset)
    {
        for (int32_t i = 0; i < m_Signatures.Range(); ++i)
            m_Signatures[i].m_Bitset = m_Bitset;
    }
    DynamicBitset::DynamicBitset(DynamicBitset&& other) noexcept
        :
        m_Signatures(std::move(other.m_Signatures)),
        m_Bitset(std::move(other.m_Bitset))
    {
    }
    DynamicBitset& DynamicBitset::operator=(DynamicBitset&& other) noexcept
    {
        m_Signatures = std::move(other.m_Signatures);
        m_Bitset = std::move(other.m_Bitset);
        return *this;
    }
    void DynamicBitset::CreateSignature()
    {
        int32_t next = m_Signatures.Next();
        if (next == m_Signatures.Range())
        {
            for (uint8_t i = 0; i < s_BitCount; ++i)
                m_Bitset.push_back(false);
        }
        m_Signatures.Emplace(s_BitCount, next, m_Bitset);
    }

    void DynamicBitset::DestroySignature(int32_t index)
    {
        m_Signatures[index].Reset();
        m_Signatures.Erase(index);
    }

    Signature& DynamicBitset::GetSignature(int32_t index)
    {
        return m_Signatures[index];
    }
    const Signature& DynamicBitset::GetSignature(int32_t index) const
    {
        return m_Signatures[index];
    }
    void DynamicBitset::SetNumberBits(uint8_t count)
    {
        uint8_t oldCount = s_BitCount;
        int16_t diff = count - oldCount;
        s_BitCount = count;


        std::vector<bool> newBitset(m_Signatures.Range() * count);
        std::fill(newBitset.begin(), newBitset.end(), false);

        uint32_t counter = 0;
        for (auto&& val : m_Bitset)
        {
            if (!(counter % oldCount))
                counter += diff;
            newBitset[counter++] = val;
        }
        m_Bitset = std::move(newBitset);
        for (int32_t i = 0; i < m_Signatures.Range(); ++i)
            m_Signatures[i] = Signature(s_BitCount, i, m_Bitset);
    }
}