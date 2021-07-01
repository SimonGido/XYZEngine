#include "stdafx.h"
#include "EditorHelper.h"


namespace XYZ {
	void EditorHelper::BeginColumns(const char* label, int count, float width)
	{
		ImGui::Columns(count);
		ImGui::PushID(label);
		ImGui::SetColumnWidth(0, width);
		ImGui::Text(label);
		ImGui::NextColumn();

	}
	void EditorHelper::EndColumns()
	{
		ImGui::PopID();
		ImGui::Columns(1);
	}
	void EditorHelper::DrawFloatControl(const char* label, const char* dragLabel, float& value, float resetValue)
	{
		ImGuiIO& io = ImGui::GetIO();
		auto boldFont = io.Fonts->Fonts[0];
		float lineHeight = GImGui->Font->FontSize + GImGui->Style.FramePadding.y * 2.0f;
		ImVec2 buttonSize = { lineHeight + 3.0f, lineHeight };

		ImGui::PushFont(boldFont);
		if (ImGui::Button(label, buttonSize))
			value = resetValue;
		ImGui::PopFont();


		ImGui::SameLine();
		ImGui::DragFloat(dragLabel, &value, 0.05f, 0.0f, 0.0f, "%.2f");
		ImGui::PopItemWidth();
	}

	void EditorHelper::DrawVec2Control(const std::string& label, glm::vec2& values, float resetValue, float columnWidth)
	{
		ImGuiIO& io = ImGui::GetIO();
		auto boldFont = io.Fonts->Fonts[0];
		BeginColumns(label.c_str());

		ImGui::PushMultiItemsWidths(2, ImGui::CalcItemWidth());
		ImGui::PushStyleVar(ImGuiStyleVar_ItemSpacing, ImVec2{ 0, 5.0f });

		ImGui::PushStyleColor(ImGuiCol_Button, ImVec4{ 0.5f, 0.5f, 0.5f, 1.0f });
		ImGui::PushStyleColor(ImGuiCol_ButtonHovered, ImVec4{ 0.6f, 0.6f, 0.6f, 1.0f });
		ImGui::PushStyleColor(ImGuiCol_ButtonActive, ImVec4{ 0.65f, 0.65f, 0.65f, 1.0f });

		DrawFloatControl("X", "##X", values.x, resetValue);
		ImGui::SameLine();
		DrawFloatControl("Y", "##Y", values.y, resetValue);
		ImGui::PopStyleColor(3);

		ImGui::PopStyleVar();

		EndColumns();
	}
	void EditorHelper::DrawVec3Control(const std::string& label, glm::vec3& values, float resetValue, float columnWidth)
	{
		ImGuiIO& io = ImGui::GetIO();
		auto boldFont = io.Fonts->Fonts[0];
		BeginColumns(label.c_str());

		ImGui::PushMultiItemsWidths(3, ImGui::CalcItemWidth());
		ImGui::PushStyleVar(ImGuiStyleVar_ItemSpacing, ImVec2{ 0, 5.0f });

		ImGui::PushStyleColor(ImGuiCol_Button, ImVec4{ 0.5f, 0.5f, 0.5f, 1.0f });
		ImGui::PushStyleColor(ImGuiCol_ButtonHovered, ImVec4{ 0.6f, 0.6f, 0.6f, 1.0f });
		ImGui::PushStyleColor(ImGuiCol_ButtonActive, ImVec4{ 0.65f, 0.65f, 0.65f, 1.0f });

		DrawFloatControl("X", "##X", values.x, resetValue);
		ImGui::SameLine();
		DrawFloatControl("Y", "##Y", values.y, resetValue);
		ImGui::SameLine();
		DrawFloatControl("Z", "##Z", values.z, resetValue);
		ImGui::PopStyleColor(3);

		ImGui::PopStyleVar();

		EndColumns();
	}
	void EditorHelper::DrawVec3ControlRGB(const std::string& label, glm::vec3& values, float resetValue, float columnWidth)
	{
		ImGuiIO& io = ImGui::GetIO();
		auto boldFont = io.Fonts->Fonts[0];
		BeginColumns(label.c_str());

		ImGui::PushMultiItemsWidths(3, ImGui::CalcItemWidth());
		ImGui::PushStyleVar(ImGuiStyleVar_ItemSpacing, ImVec2{ 0, 5.0f });

		ImGui::PushStyleColor(ImGuiCol_Button, ImVec4{ 0.5f, 0.5f, 0.5f, 1.0f });
		ImGui::PushStyleColor(ImGuiCol_ButtonHovered, ImVec4{ 0.6f, 0.6f, 0.6f, 1.0f });
		ImGui::PushStyleColor(ImGuiCol_ButtonActive, ImVec4{ 0.65f, 0.65f, 0.65f, 1.0f });

		DrawFloatControl("R", "##R", values.x, resetValue);
		ImGui::SameLine();
		DrawFloatControl("G", "##G", values.y, resetValue);
		ImGui::SameLine();
		DrawFloatControl("B", "##B", values.z, resetValue);
		ImGui::PopStyleColor(3);

		ImGui::PopStyleVar();

		EndColumns();
	}
	void EditorHelper::DrawVec4Control(const std::string& label, const char* names[4], glm::vec4& values, float resetValue, float columnWidth)
	{
		ImGuiIO& io = ImGui::GetIO();
		auto boldFont = io.Fonts->Fonts[0];

		BeginColumns(label.c_str(), 2, columnWidth);

		ImGui::PushMultiItemsWidths(4, ImGui::CalcItemWidth());
		ImGui::PushStyleVar(ImGuiStyleVar_ItemSpacing, ImVec2{ 0, 5.0f });

		ImGui::PushStyleColor(ImGuiCol_Button, ImVec4{ 0.5f, 0.5f, 0.5f, 1.0f });
		ImGui::PushStyleColor(ImGuiCol_ButtonHovered, ImVec4{ 0.6f, 0.6f, 0.6f, 1.0f });
		ImGui::PushStyleColor(ImGuiCol_ButtonActive, ImVec4{ 0.65f, 0.65f, 0.65f, 1.0f });

		DrawFloatControl(names[0], "##R", values.r, resetValue);
		ImGui::SameLine();
		DrawFloatControl(names[1], "##G", values.g, resetValue);
		ImGui::SameLine();
		DrawFloatControl(names[2], "##B", values.b, resetValue);
		ImGui::SameLine();
		DrawFloatControl(names[3], "##A", values.a, resetValue);


		ImGui::PopStyleColor(3);
		ImGui::PopStyleVar();
		EndColumns();
	}
	void EditorHelper::DrawColorControl(const std::string& label, glm::vec4& values, float resetValue, float columnWidth)
	{
		ImGuiIO& io = ImGui::GetIO();
		auto boldFont = io.Fonts->Fonts[0];

		BeginColumns(label.c_str());

		ImGui::PushMultiItemsWidths(4, ImGui::CalcItemWidth());
		ImGui::PushStyleVar(ImGuiStyleVar_ItemSpacing, ImVec2{ 0, 5.0f });

		ImGui::PushStyleColor(ImGuiCol_Button, ImVec4{ 0.5f, 0.5f, 0.5f, 1.0f });
		ImGui::PushStyleColor(ImGuiCol_ButtonHovered, ImVec4{ 0.6f, 0.6f, 0.6f, 1.0f });
		ImGui::PushStyleColor(ImGuiCol_ButtonActive, ImVec4{ 0.65f, 0.65f, 0.65f, 1.0f });

		DrawFloatControl("R", "##R", values.r, resetValue);
		ImGui::SameLine();
		DrawFloatControl("G", "##G", values.g, resetValue);
		ImGui::SameLine();
		DrawFloatControl("B", "##B", values.b, resetValue);
		ImGui::SameLine();
		DrawFloatControl("A", "##A", values.a, resetValue);


		ImGui::PopStyleColor(3);
		ImGui::PopStyleVar();
		EndColumns();
	}
	void EditorHelper::DrawSplitter(bool splitHorizontally, float thickness, float* size0, float* size1, float minSize0, float minSize1)
	{
		ImVec2 backup_pos = ImGui::GetCursorPos();
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
}