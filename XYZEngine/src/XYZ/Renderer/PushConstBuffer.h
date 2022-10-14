#pragma once
#include <stdint.h>
#include <memory>

namespace XYZ {
	struct PushConstBuffer
	{
		PushConstBuffer();

		template <typename ...Args>
		PushConstBuffer(const Args&... args)
		{
			constexpr size_t size = (sizeof(Args) + ...);
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