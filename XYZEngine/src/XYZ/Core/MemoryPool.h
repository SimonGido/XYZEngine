#pragma once

namespace XYZ {

	class MemoryPool
	{
	private:
		struct Block
		{
			uint8_t* Data = nullptr;
			uint32_t NextAvailableIndex = 0;
		};
		struct Chunk
		{
			uint32_t Size = 0;
			uint32_t ChunkIndex = 0;
			uint8_t  BlockIndex = 0;
		};

	public:
		MemoryPool(size_t blockSize);
		~MemoryPool();

		template <typename T, typename ...Args>
		T* Allocate(Args&&... args)
		{
			if (!m_Blocks.size())
			{
				m_Blocks.push_back(Block());
				m_Blocks.back().Data = new uint8_t[m_BlockSize];
				memset(m_Blocks.back().Data, 0, m_BlockSize);
			}
			auto [blockIndex, chunkIndex] = findAvailableIndex((uint32_t)sizeof(T));
			return new((void*)&m_Blocks[blockIndex].Data[chunkIndex])T(std::forward<Args>(args)...);
		}

		template <typename T>
		void Deallocate(T* val)
		{
			val->~T();
			uint8_t* tmp = (uint8_t*)val + sizeof(T);
			uint8_t blockIndex = *tmp;
			uint32_t chunkIndex = *(uint32_t*)(tmp + sizeof(uint8_t));

			Chunk chunk{ sizeof(T) + sizeof(uint32_t) + sizeof(uint8_t), chunkIndex, blockIndex };
			// Removed chunk is at the end of used memory so just move available index backwards
			if (chunk.ChunkIndex + chunk.Size == m_Blocks[blockIndex].NextAvailableIndex)
				m_Blocks[blockIndex].NextAvailableIndex -= chunk.ChunkIndex + chunk.BlockIndex + chunk.Size;
			else
			{
				m_FreeChunks.push_back(chunk);
				//mergeFreeChunks();
			}
		}
	private:
		void mergeFreeChunks();
		std::pair<uint8_t, uint32_t> findAvailableIndex(uint32_t size);

	private:
		const size_t m_BlockSize;

		// TODO: Multiple blocks
		std::vector<Block> m_Blocks;

		std::vector<Chunk> m_FreeChunks;
	};
}