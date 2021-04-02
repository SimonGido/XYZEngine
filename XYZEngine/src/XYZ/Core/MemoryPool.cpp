#include "stdafx.h"
#include "MemoryPool.h"


namespace XYZ {

	namespace Helper {
		size_t NumBytesToStore(size_t size)
		{
			return (size_t)std::ceil((log(size) / log(2)) / 4.0f);
		}
	}

	MemoryPool::MemoryPool(size_t blockSize)
		:
		m_BlockSize(blockSize),
		m_SizeOfChunkIndex(Helper::NumBytesToStore(blockSize))
	{
	}

	MemoryPool::~MemoryPool()
	{
		for (auto& block : m_Blocks)
		{
			if (block.Data)
				delete[]block.Data;
		}
	}

	void MemoryPool::cleanUp()
	{
		sortFreeChunks();
		mergeFreeChunks();
		reverseMergeFreeChunks();
		m_Dirty = false;
	}

	void MemoryPool::sortFreeChunks()
	{
		std::sort(m_FreeChunks.begin(), m_FreeChunks.end(), [](const Chunk& a, const Chunk& b) {
			if (a.BlockIndex == b.BlockIndex)
				return a.ChunkIndex < b.ChunkIndex;
			return a.BlockIndex < b.BlockIndex;
			});
	}

	void MemoryPool::mergeFreeChunks()
	{		
		for (auto it = m_FreeChunks.begin(); it != m_FreeChunks.end(); ++it)
		{
			for (auto it2 = it + 1; it2 != m_FreeChunks.end();)
			{
				// Not from the same block
				if (it->BlockIndex != it2->BlockIndex)
					break;
				// Chunks are connected so merge them
				if (it->ChunkIndex + it->Size == it2->ChunkIndex || it2->ChunkIndex + it2->Size == it->ChunkIndex)
				{
					it->ChunkIndex = std::min(it2->ChunkIndex, it->ChunkIndex);
					it->Size = it->Size + it2->Size;
					it2 = m_FreeChunks.erase(it2);
					continue;
				}
				else
				{
					// Chunks are not connected after their indices where sort so continue to check other chunks
					break;
				}
				it2++;
			}
		}
	}
	void MemoryPool::reverseMergeFreeChunks()
	{
		for (int64_t i = m_FreeChunks.size() - 1; i >= 0; --i)
		{
			Chunk& chunk = m_FreeChunks[i];
			Block& block = m_Blocks[chunk.BlockIndex];
			size_t chunkSpace = (size_t)chunk.ChunkIndex + (size_t)chunk.Size;
			if (chunkSpace != block.NextAvailableIndex)
				break;
			block.NextAvailableIndex -= (size_t)chunk.Size;
			m_FreeChunks.erase(m_FreeChunks.begin() + i);
		}
	}
	MemoryPool::Block* MemoryPool::createBlock()
	{
		m_Blocks.push_back(Block());
		m_Blocks.back().Data = new uint8_t[m_BlockSize];
		memset(m_Blocks.back().Data, 0, m_BlockSize);
		return &m_Blocks.back();
	}
	std::pair<uint8_t, uint32_t> MemoryPool::findAvailableIndex(uint32_t size)
	{
		uint32_t counter = 0;
		size_t spaceRequirement = size + m_SizeOfChunkIndex + sizeof(uint8_t);
		for (auto& chunk : m_FreeChunks)
		{
			if (chunk.Size > spaceRequirement)
			{
				Chunk tmp = chunk;
				chunk.ChunkIndex += spaceRequirement;
				chunk.Size -= spaceRequirement;
				return { tmp.BlockIndex, tmp.ChunkIndex };
			}
			else if (chunk.Size == spaceRequirement)
			{
				Chunk tmp = chunk;
				m_FreeChunks.erase(m_FreeChunks.begin() + counter);
				return { tmp.BlockIndex, tmp.ChunkIndex };
			}
			counter++;
		}

		Block* last = &m_Blocks.back();
		if ((size_t)last->NextAvailableIndex + spaceRequirement > m_BlockSize)
			last = createBlock();


		uint32_t chunkIndex = last->NextAvailableIndex;
		uint8_t blockIndex = (uint8_t)m_Blocks.size() - 1;
		last->NextAvailableIndex += spaceRequirement;
		size_t blockIndexInMemory = (size_t)size + (size_t)chunkIndex;
		size_t chunkIndexInMemory = (size_t)size + (size_t)chunkIndex + sizeof(uint8_t);

		memcpy(&m_Blocks[blockIndex].Data[blockIndexInMemory], &blockIndex, sizeof(uint8_t));	// Store block index
		memcpy(&m_Blocks[blockIndex].Data[chunkIndexInMemory], &chunkIndex, (size_t)m_SizeOfChunkIndex);	// Store chunk index

		return { blockIndex, chunkIndex };
	}
}