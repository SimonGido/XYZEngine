#include "stdafx.h"
#include "MemoryPool.h"


namespace XYZ {

	MemoryPool::MemoryPool(size_t blockSize)
		:
		m_BlockSize(blockSize)
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

	void MemoryPool::mergeFreeChunks()
	{
		std::sort(m_FreeChunks.begin(), m_FreeChunks.end(), [](const Chunk& a, const Chunk& b) {
			if (a.BlockIndex == b.BlockIndex)
				return a.ChunkIndex < b.ChunkIndex;
			return a.BlockIndex < b.BlockIndex;
			});

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
	std::pair<uint8_t, uint32_t> MemoryPool::findAvailableIndex(uint32_t size)
	{
		uint32_t counter = 0;
		size_t spaceRequirement = size + sizeof(uint32_t) + sizeof(uint8_t);
		for (auto& chunk : m_FreeChunks)
		{
			if (chunk.Size >= spaceRequirement)
			{
				Chunk tmp = chunk;
				m_FreeChunks.erase(m_FreeChunks.begin() + counter);
				return { tmp.BlockIndex, tmp.ChunkIndex };
			}
			else if (counter == m_FreeChunks.size() - 1)
			{
				Block& block = m_Blocks[chunk.BlockIndex];
				// If there is not occupied memory after the last free chunk, use the last chunk space together with not occupied memory
				if ((size_t)chunk.ChunkIndex + size == block.NextAvailableIndex && (size_t)chunk.ChunkIndex + size < m_BlockSize)
				{
					Chunk tmp = chunk;
					m_FreeChunks.erase(m_FreeChunks.begin() + counter);
					block.NextAvailableIndex = tmp.ChunkIndex + size;
					return { tmp.BlockIndex, tmp.ChunkIndex };
				}
			}
			counter++;
		}

		Block* last = &m_Blocks.back();
		if ((size_t)last->NextAvailableIndex + spaceRequirement > m_BlockSize)
		{
			m_Blocks.push_back(Block());
			last = &m_Blocks.back();
			last->Data = new uint8_t[m_BlockSize];
			memset(last->Data, 0, m_BlockSize);
		}
		uint32_t chunkIndex = last->NextAvailableIndex;
		uint8_t blockIndex = (uint8_t)m_Blocks.size() - 1;
		last->NextAvailableIndex += spaceRequirement + 1;
		size_t blockIndexInMemory = (size_t)size + (size_t)chunkIndex;
		size_t chunkIndexInMemory = (size_t)size + (size_t)chunkIndex + sizeof(uint8_t);
		
		memcpy(&m_Blocks[blockIndex].Data[blockIndexInMemory], &blockIndex, sizeof(uint8_t));	// Store block index
		memcpy(&m_Blocks[blockIndex].Data[chunkIndexInMemory], &chunkIndex, sizeof(uint32_t));	// Store chunk index

		return { blockIndex, chunkIndex };
	}
}