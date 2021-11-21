#pragma once
#include "XYZ/Core/Core.h"
#include "XYZ/Core/Assert.h"

namespace XYZ {

	class ByteBuffer
	{
	public:
		ByteBuffer()
			: m_Data(nullptr), m_Size(0)
		{}

		ByteBuffer(uint8_t* data, uint32_t size)
			: m_Data(data), m_Size(size)
		{}
		void Allocate(uint32_t size)
		{
			delete[] m_Data;
			m_Data = nullptr;

			if (size == 0)
				return;

			m_Data = new uint8_t[size];
			m_Size = size;
		}
		void Destroy()
		{
			Allocate(0);
		}

		void ZeroInitialize()
		{
			if (m_Data)
				memset(m_Data, 0, m_Size);
		}

		void Write(const void* data, uint32_t size, uint32_t offset = 0)
		{
			XYZ_ASSERT(offset + size <= m_Size, "Buffer overflow!");
			memcpy(m_Data + offset, data, size);
		}

		template<typename T>
		T& Read(uint32_t offset = 0)
		{
			return *(T*)((uint8_t*)m_Data + offset);
		}
		operator bool() const
		{
			return m_Data;
		}

		uint8_t& operator[](int index)
		{
			return m_Data[index];
		}

		uint8_t operator[](int index) const
		{
			return m_Data[index];
		}

		operator uint8_t* ()
		{
			return m_Data;
		}

		operator uint8_t* () const
		{
			return m_Data;
		}

		ByteBuffer& operator=(uint8_t* data)
		{
			m_Data = data;
			return *this;
		}



		template<typename T>
		T* As()
		{
			return (T*)m_Data;
		}

		inline uint32_t GetSize() const { return m_Size; }


		static ByteBuffer Copy(const void* data, uint32_t size)
		{
			ByteBuffer buffer;
			buffer.Allocate(size);

			memcpy(buffer.m_Data, data, size);
			return buffer;
		}


		uint32_t m_Size;
		uint8_t* m_Data;
	};
}