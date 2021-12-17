#include "stdafx.h"
#include "MemoryPoolView.h"

#include "XYZ/ImGui/ImGui.h"

namespace XYZ {
	namespace Editor {
		MemoryPoolView::MemoryPoolView(const MemoryPool& pool)
			:
			m_Pool(pool)
		{
		}
		void MemoryPoolView::OnImGuiRender()
		{	
			XYZ_ASSERT(ImGui::GetCurrentWindow(), "ImGui must have active window");
			if (ImGui::BeginTable("##MemoryStats", 2, ImGuiTableFlags_SizingFixedFit))
			{
				UI::TextTableRow("%s", "Block Size: ", "%u", m_Pool.GetBlockSize());
				UI::TextTableRow("%s", "Block Count: ", "%u", m_Pool.GetNumBlocks());
				UI::TextTableRow("%s", "Memory Used: ", "%u", m_Pool.GetMemoryUsed());
				UI::TextTableRow("%s", "Allocation Count: ", "%u", m_Pool.GetNumAllocations());

				ImGui::EndTable();
			}
			
			if (UI::BeginTreeNode("Allocations"))
			{
				if (ImGui::BeginTable("##AllocNames", 2, ImGuiTableFlags_SizingFixedFit))
				{
					const auto& allocations = m_Pool.GetAllocations();
					for (int32_t i = 0; i < allocations.Range(); ++i)
					{
						if (allocations.Valid(i) && allocations[i])
							UI::TextTableRow("%s", "Allocation:", "%s", allocations[i]);
					}
					ImGui::EndTable();
				}
				UI::EndTreeNode();
			}
			if (UI::BeginTreeNode("Free Chunks"))
			{
				if (ImGui::BeginTable("##FreeChunks", 3, ImGuiTableFlags_SizingFixedFit))
				{
					const auto& freeChunks = m_Pool.GetFreeChunks();
					for (const auto& chunk : freeChunks)
					{
						// TODO:
					}
					ImGui::EndTable();
				}

				UI::EndTreeNode();
			}		
		}
	}
}