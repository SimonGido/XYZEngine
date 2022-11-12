#pragma once
#include "XYZ/Utils/Math/Math.h"

#include <stdint.h>
#include <memory>

namespace XYZ {
	struct XYZ_API PushConstBuffer
	{
		PushConstBuffer();

		template <typename ...Args>
		PushConstBuffer(const Args&... args)
		{
			constexpr size_t size = (sizeof(Args) + ...);
			XYZ_ASSERT(size < sc_MaxSize, "");

			size_t offset = 0;
			(store(args, offset), ...);
			Size = size;
		}
		PushConstBuffer(const PushConstBuffer& other)
		{
			memcpy(Bytes, other.Bytes, other.Size);
			Size = other.Size;
		}
		PushConstBuffer& operator=(const PushConstBuffer& other)
		{
			memcpy(Bytes, other.Bytes, other.Size);
			Size = other.Size;
			return *this;
		}

		PushConstBuffer& operator+=(const PushConstBuffer& other)
		{
			XYZ_ASSERT(other.Size + Size < sc_MaxSize, "");
			memcpy(&Bytes[Size], other.Bytes, other.Size);
			Size += other.Size;
			return *this;
		}

		PushConstBuffer operator+(const PushConstBuffer& other) const
		{
			PushConstBuffer result(*this);
			XYZ_ASSERT(other.Size + result.Size < sc_MaxSize, "");
			memcpy(&result.Bytes[result.Size], other.Bytes, other.Size);
			result.Size += other.Size;
			return result;
		}

		static constexpr size_t sc_MaxSize = 128;

		std::byte Bytes[sc_MaxSize];
		uint32_t  Size = 0;

	private:
		template <typename T>
		void store(const T& val, size_t& offset)
		{
			memcpy(&Bytes[offset], &val, sizeof(T));
			offset += sizeof(T);
		}
	};
}