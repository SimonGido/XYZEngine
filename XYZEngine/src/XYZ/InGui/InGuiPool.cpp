#include "stdafx.h"
#include "InGuiPool.h"


namespace XYZ {
	InGuiPool::InGuiPool(size_t chunkCapacity)
		: m_Capacity(chunkCapacity)
	{
		createNewChunk();
	}
	InGuiPool::~InGuiPool()
	{
		for (auto& chunk : m_Chunks)
			delete[] chunk.Data;
		for (auto& chunk : m_EmptyChunks)
			delete[] chunk.Data;
	}
	void InGuiPool::createNewChunk()
	{
		if (m_EmptyChunks.size())
		{
			m_Chunks.push_back(m_EmptyChunks.back());
			m_EmptyChunks.pop_back();
		}
		else
		{
			uint8_t* data = new uint8_t[m_Capacity];
			m_Chunks.emplace_back(data);
		}
	}
}