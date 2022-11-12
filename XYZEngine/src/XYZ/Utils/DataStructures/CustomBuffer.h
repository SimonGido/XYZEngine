#pragma once
#include "XYZ/Core/Core.h"

namespace XYZ {
    class XYZ_API CustomBuffer
    {
    public:
        CustomBuffer();
        CustomBuffer(size_t elementSize, size_t size);
        CustomBuffer(const CustomBuffer& other);
        CustomBuffer(CustomBuffer&& other) noexcept;
        ~CustomBuffer();

        CustomBuffer& operator=(const CustomBuffer& other);
        CustomBuffer& operator=(CustomBuffer&& other) noexcept;

        void Resize(size_t count);
        void SetElementSize(size_t size);
        void Clear();

        template <typename T>
        T* As();

        template <typename T>
        const T* As() const;

        operator std::byte* () { return m_Data; }
        operator std::byte* () const { return m_Data; }

        size_t Count() const { return m_Count; }
        size_t ElementSize() const { return m_ElementSize; }
        size_t SizeInBytes() const { return m_Count * m_ElementSize; }
    private:
        std::byte* m_Data;
        size_t     m_ElementSize;
        size_t	   m_Count;
    };

    template <typename T>
    T* CustomBuffer::As()
    {
        XYZ_ASSERT(m_ElementSize == sizeof(T), "Different element size");
        return reinterpret_cast<T*>(m_Data);
    }

    template <typename T>
    const T* CustomBuffer::As() const
    {
        XYZ_ASSERT(m_ElementSize == sizeof(T), "Different element size");
        return reinterpret_cast<T*>(m_Data);
    }

}