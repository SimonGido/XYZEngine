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
				createBlock();
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

			Chunk chunk{ sizeof(T) + m_ChunkIndexSize + sizeof(uint8_t), chunkIndex, blockIndex };
			m_FreeChunks.push_back(chunk);
			mergeFreeChunks();
			reverseMergeFreeChunks();
		}

		template <typename T>
		T* Get(uint32_t chunkIndex, uint8_t blockIndex)
		{
			return reinterpret_cast<T*>(&m_Blocks[blockIndex].Data[chunkIndex]);
		}

		template <typename T>
		std::pair<uint32_t, uint8_t> ExtractIndices(T* ptr) const
		{
			uint8_t* tmp = (uint8_t*)ptr + sizeof(T);
			uint8_t blockIndex = *tmp;
			uint32_t chunkIndex = *(uint32_t*)(tmp + sizeof(uint8_t));
			return { chunkIndex, blockIndex };
		}

	private:
		void mergeFreeChunks();
		void reverseMergeFreeChunks();
		Block* createBlock();

		std::pair<uint8_t, uint32_t> findAvailableIndex(uint32_t size);

	private:
		const size_t m_BlockSize;
		const size_t m_ChunkIndexSize;

		std::vector<Block> m_Blocks;
		std::vector<Chunk> m_FreeChunks;
	};
}