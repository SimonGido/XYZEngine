#include "stdafx.h"
#include "DynamicBitset.h"


namespace XYZ {

    DynamicBitset::DynamicBitset(const DynamicBitset& other)
        :
        m_Signatures(other.m_Signatures),
        m_Bitset(other.m_Bitset),
        m_BitCount(other.m_BitCount)
    {
        for (int32_t i = 0; i < m_Signatures.Range(); ++i)
            m_Signatures[i].m_Bitset = this;
    }
    DynamicBitset::DynamicBitset(DynamicBitset&& other) noexcept
        :
        m_Signatures(std::move(other.m_Signatures)),
        m_Bitset(std::move(other.m_Bitset)),
        m_BitCount(other.m_BitCount)
    {
    }
    DynamicBitset& DynamicBitset::operator=(DynamicBitset&& other) noexcept
    {
        m_Signatures = std::move(other.m_Signatures);
        m_Bitset = std::move(other.m_Bitset);
        return *this;
    }
    int32_t DynamicBitset::CreateSignature()
    {
        int32_t next = m_Signatures.Next();
        if (next == m_Signatures.Range())
        {
            for (uint8_t i = 0; i < m_BitCount; ++i)
                m_Bitset.push_back(false);
        }
        return m_Signatures.Emplace(next, this);
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
    void DynamicBitset::SetNumberBits(uint16_t count)
    {
        if (m_BitCount == count)
            return;
        uint16_t oldCount = m_BitCount;
        uint16_t diff = count - oldCount;
        m_BitCount = count;

        std::vector<bool> newBitset((size_t)m_Signatures.Range() * count);
        std::fill(newBitset.begin(), newBitset.end(), false);

        // Remap bits
        uint32_t counter = 0;
        uint32_t multiplier = 0;
        for (size_t i = 0; i < m_Bitset.size(); ++i)
        {
            if (counter == oldCount)
            {
                counter = 0;
                multiplier++;
            }
            uint32_t index = (multiplier * count) + counter;
            newBitset[index] = m_Bitset[i];
            counter++;
        }
        m_Bitset = std::move(newBitset);
    }
    void DynamicBitset::Clear()
    {
        m_Bitset.clear();
        CreateSignature();
    }
    Signature& DynamicBitset::operator[](int32_t index)
    {
        return m_Signatures[index];
    }
    const Signature& DynamicBitset::operator[](int32_t index) const
    {
        return m_Signatures[index];
    }
}