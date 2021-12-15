#include "stdafx.h"
#include "EditorHelper.h"

#include "XYZ/ImGui/ImGui.h"

namespace XYZ {
	
	void EditorHelper::DrawSplitter(bool splitHorizontally, float thickness, float* size0, float* size1, float minSize0, float minSize1)
	{
		const ImVec2 backup_pos = ImGui::GetCursorPos();
		if (splitHorizontally)
			ImGui::SetCursorPosY(backup_pos.y + *size0);
		else
			ImGui::SetCursorPosX(backup_pos.x + *size0);

		ImGui::PushStyleColor(ImGuiCol_Button, ImVec4(0, 0, 0, 0));
		ImGui::PushStyleColor(ImGuiCol_ButtonActive, ImVec4(0, 0, 0, 0));          // We don't draw while active/pressed because as we move the panes the splitter button will be 1 frame late
		ImGui::PushStyleColor(ImGuiCol_ButtonHovered, ImVec4(0.6f, 0.6f, 0.6f, 0.10f));
		ImGui::Button("##Splitter", ImVec2(!splitHorizontally ? thickness : -1.0f, splitHorizontally ? thickness : -1.0f));
		ImGui::PopStyleColor(3);

		ImGui::SetItemAllowOverlap(); // This is to allow having other buttons OVER our splitter. 

		if (ImGui::IsItemActive())
		{
			float mouse_delta = splitHorizontally ? ImGui::GetIO().MouseDelta.y : ImGui::GetIO().MouseDelta.x;

			// Minimum pane size
			if (mouse_delta < minSize0 - *size0)
				mouse_delta = minSize0 - *size0;
			if (mouse_delta > *size1 - minSize1)
				mouse_delta = *size1 - minSize1;

			// Apply resize
			*size0 += mouse_delta;
			*size1 -= mouse_delta;
		}
		ImGui::SetCursorPos(backup_pos);
	}
	void EditorHelper::PushDisabled()
	{
		ImGuiContext& g = *GImGui;
		if ((g.CurrentItemFlags & ImGuiItemFlags_Disabled) == 0)
			ImGui::PushStyleVar(ImGuiStyleVar_Alpha, g.Style.Alpha * 0.6f);
		ImGui::PushItemFlag(ImGuiItemFlags_Disabled, true);
	}
	void EditorHelper::PopDisabled()
	{
		ImGuiContext& g = *GImGui;
		ImGui::PopItemFlag();
		if ((g.CurrentItemFlags & ImGuiItemFlags_Disabled) == 0)
			ImGui::PopStyleVar();
	}
}