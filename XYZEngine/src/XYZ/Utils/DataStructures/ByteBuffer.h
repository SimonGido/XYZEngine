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
}