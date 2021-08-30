#pragma once

#include "XYZ/Utils/DataStructures/MemoryPool.h"

#include <imgui.h>

namespace XYZ {

	template <uint32_t BlockSize, bool StoreSize = false>
	class MemoryPoolDebug
	{
	public:
		MemoryPoolDebug();

		void SetContext(MemoryPool<BlockSize, StoreSize>* pool);
		void OnImGuiRender();

	private:
		MemoryPool<BlockSize, StoreSize>* m_MemoryPool;
	};
	template<uint32_t BlockSize, bool StoreSize>
	inline MemoryPoolDebug<BlockSize, StoreSize>::MemoryPoolDebug()
		:
		m_MemoryPool(nullptr)
	{
	}
	template<uint32_t BlockSize, bool StoreSize>
	inline void MemoryPoolDebug<BlockSize, StoreSize>::SetContext(MemoryPool<BlockSize, StoreSize>* pool)
	{
		m_MemoryPool = pool;
	}
	template<uint32_t BlockSize, bool StoreSize>
	inline void MemoryPoolDebug<BlockSize, StoreSize>::OnImGuiRender()
	{
		if (ImGui::Begin("Memory Pool"))
		{
			if (m_MemoryPool)
			{
				ImGui::Text("Element Count: %u", m_MemoryPool->m_ElementCounter);
				ImGui::Text("Memory Used: %u kb", m_MemoryPool->m_MemoryUsed);
				ImGui::Text("Free Chunks:");
				for (const auto& freeChunk : m_MemoryPool->m_FreeChunks)
				{
					ImGui::Text("Size: %u  Chunk Index: %u  Block Index: %u", freeChunk.Size, freeChunk.ChunkIndex, freeChunk.BlockIndex);
				}
			}
		}
		ImGui::End();
	}
}