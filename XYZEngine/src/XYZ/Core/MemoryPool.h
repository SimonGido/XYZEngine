#pragma once
#include <cstdint>
#include <vector>

namespace XYZ {

	class MemoryPool
	{
	private:
		struct Block
		{
			uint8_t* Data = nullptr;
		};
		struct Chunk
		{
			size_t Size = 0;
			size_t Index = 0;
		};

	public:
		MemoryPool(size_t blockSize);
		~MemoryPool();

		template <typename T, typename ...Args>
		T* Allocate(Args&&... args)
		{
			if (!m_Block.Data)
			{
				m_Block.Data = new uint8_t[m_BlockSize];
				memset(m_Block.Data, 0, m_BlockSize);
			}
			size_t index = findAvailableIndex(sizeof(T));
			return new((void*)&m_Block.Data[index])T(std::forward<Args>(args)...);
		}

		template <typename T>
		void Deallocate(T* val)
		{
			val->~T();
			uint8_t* tmp = (uint8_t*)val + sizeof(T);
			size_t index = *(size_t*)tmp;

			Chunk chunk{ sizeof(T) + sizeof(size_t), index };
			// Removed chunk is at the end of used memory so just move available index backwards
			if (chunk.Index + chunk.Size == m_NextAvailableIndex)
				m_NextAvailableIndex -= chunk.Index + chunk.Size;
			else
			{
				m_FreeChunks.push_back(chunk);
				mergeFreeChunks();
			}
		}
	private:
		void mergeFreeChunks();
		size_t findAvailableIndex(size_t size);

	private:
		const size_t m_BlockSize;

		size_t m_NextAvailableIndex = 0;

		// TODO: Multiple blocks
		Block m_Block;

		std::vector<Chunk> m_FreeChunks;
	};
}