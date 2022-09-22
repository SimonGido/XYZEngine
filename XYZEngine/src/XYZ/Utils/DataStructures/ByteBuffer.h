#pragma once
#include "XYZ/Core/Core.h"
#include "XYZ/Core/Assert.h"

namespace XYZ {

	class ByteBuffer
	{
	public:
		ByteBuffer()
			: Data(nullptr), Size(0)
		{}

		ByteBuffer(uint8_t* data, uint32_t size)
			: Data(data), Size(size)
		{}
		
		void Allocate(uint32_t size)
		{
			delete[] Data;
			Data = nullptr;

			if (size == 0)
				return;

			Data = new uint8_t[size];
			Size = size;
		}

		bool TryReallocate(uint32_t size)
		{
			if (size > Size)
			{
				uint8_t * newData = new uint8_t[size];
				memcpy(newData, Data, std::min(Size, size));
				Size = size;
				delete[] Data;
				Data = newData;
				return true;
			}
			return false;
		}

		void Destroy()
		{
			Allocate(0);
		}

		void ZeroInitialize()
		{
			if (Data)
				memset(Data, 0, Size);
		}

		void Write(const void* data, uint32_t size, uint32_t offset = 0)
		{
			XYZ_ASSERT(offset + size <= Size, "Buffer overflow!");
			memcpy(Data + offset, data, size);
		}

		template <typename T>
		uint32_t Write(const T& data, uint32_t offset = 0)
		{
			XYZ_ASSERT(offset + sizeof(T) <= Size, "Buffer overflow!");
			new (&Data[offset])T(data);
			return static_cast<uint32_t>(sizeof(T));
		}


		template<typename T>
		T& Read(uint32_t offset = 0)
		{
			return *(T*)((uint8_t*)Data + offset);
		}

		operator bool() const
		{
			return Data;
		}

		uint8_t& operator[](int index)
		{
			return Data[index];
		}

		uint8_t operator[](int index) const
		{
			return Data[index];
		}

		operator uint8_t* ()
		{
			return Data;
		}

		operator uint8_t* () const
		{
			return Data;
		}

		ByteBuffer& operator=(uint8_t* data)
		{
			Data = data;
			return *this;
		}


		template<typename T>
		T* As()
		{
			return (T*)Data;
		}

		ByteBuffer Copy(uint32_t size) const
		{
			ByteBuffer buffer;
			buffer.Allocate(size);

			memcpy(buffer.Data, Data, size);
			return buffer;
		}

		static ByteBuffer Copy(const void* data, uint32_t size)
		{
			ByteBuffer buffer;
			buffer.Allocate(size);

			memcpy(buffer.Data, data, size);
			return buffer;
		}


		uint32_t Size;
		uint8_t* Data;
	};


	template <size_t Alignment>
	class BufferWriter
	{
	public:
		template <typename T>
		void Write(const T& val)
		{
			constexpr size_t size = sizeof(T);
			constexpr size_t missingBytes = size % Alignment;

			m_Buffer.TryReallocate(m_Offset + size + missingBytes);
			m_Buffer.Write(val, m_Offset);

			m_Offset += size;
			if constexpr (missingBytes != 0)
			{
				m_Offset += Alignment - missingBytes;
			}
		}

		template <typename T>
		void WriteOffset(uint32_t offset, const T& val)
		{
			m_Buffer.TryReallocate(m_Offset + sizeof(val));
			m_Buffer.Write(val, offset);
		}

		void Reset(uint32_t offset = 0)
		{
			m_Offset = offset;
		}

		const ByteBuffer& GetBuffer() const { return m_Buffer; }
		uint32_t		  GetOffset() const { return m_Offset; }
	private:
		ByteBuffer m_Buffer;
		uint32_t   m_Offset = 0;
	};
}