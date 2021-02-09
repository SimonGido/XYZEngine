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
		if (m_Block.Data)
			delete[]m_Block.Data;
	}

	void MemoryPool::mergeFreeChunks()
	{
		std::sort(m_FreeChunks.begin(), m_FreeChunks.end(), [](const Chunk& a, const Chunk& b) {
			return a.Index < b.Index;
			});
		for (auto it = m_FreeChunks.begin(); it != m_FreeChunks.end(); ++it)
		{
			for (auto it2 = it + 1; it2 != m_FreeChunks.end();)
			{
				// Chunks are connected so merge them
				if (it->Index + it->Size == it2->Index || it2->Index + it2->Size == it->Index)
				{
					it->Index = std::min(it2->Index, it->Index);
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
	size_t MemoryPool::findAvailableIndex(size_t size)
	{
		uint32_t counter = 0;
		for (auto& chunk : m_FreeChunks)
		{
			if (chunk.Size >= size + sizeof(size_t))
			{
				Chunk tmp = chunk;
				m_FreeChunks.erase(m_FreeChunks.begin() + counter);
				return tmp.Index;
			}
			else if (counter == m_FreeChunks.size() - 1)
			{
				// If there is not occupied memory after the last free chunk, uset he last chunk space together with not occupied memory
				if (chunk.Index + size == m_NextAvailableIndex && chunk.Index + size < m_BlockSize)
				{
					Chunk tmp = chunk;
					m_FreeChunks.erase(m_FreeChunks.begin() + counter);
					m_NextAvailableIndex = tmp.Index + size;
					return tmp.Index;
				}
			}
			counter++;
		}

		size_t index = m_NextAvailableIndex;
		m_NextAvailableIndex += size + sizeof(size_t);
		*(size_t*)&m_Block.Data[index + size] = index; // Store into user data index of the chunk

		return index;
	}
}