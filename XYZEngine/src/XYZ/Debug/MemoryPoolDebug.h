#pragma once

#include "XYZ/Utils/DataStructures/MemoryPool.h"

#include <imgui.h>

namespace XYZ {

	class MemoryPoolDebug
	{
	public:
		MemoryPoolDebug();

		void SetContext(MemoryPool* pool);
		void OnImGuiRender();

	private:
		MemoryPool* m_MemoryPool;
		float		m_AverageAllocationTime;
	};

	inline MemoryPoolDebug::MemoryPoolDebug()
		:
		m_MemoryPool(nullptr)
	{
	}

	inline void MemoryPoolDebug::SetContext(MemoryPool* pool)
	{
		m_MemoryPool = pool;
	}

	inline void MemoryPoolDebug::OnImGuiRender()
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