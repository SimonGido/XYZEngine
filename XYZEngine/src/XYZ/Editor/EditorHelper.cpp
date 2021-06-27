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
	void EditorHelper::DrawFloatControl(const char* label, const char* dragLabel, float& value, float resetValue, float columnWidth)
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

		DrawFloatControl("X", "##X", values.x, resetValue, columnWidth);
		ImGui::SameLine();
		DrawFloatControl("Y", "##Y", values.y, resetValue, columnWidth);
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

		DrawFloatControl("X", "##X", values.x, resetValue, columnWidth);
		ImGui::SameLine();
		DrawFloatControl("Y", "##Y", values.y, resetValue, columnWidth);
		ImGui::SameLine();
		DrawFloatControl("Z", "##Z", values.z, resetValue, columnWidth);
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

		DrawFloatControl("R", "##R", values.x, resetValue, columnWidth);
		ImGui::SameLine();
		DrawFloatControl("G", "##G", values.y, resetValue, columnWidth);
		ImGui::SameLine();
		DrawFloatControl("B", "##B", values.z, resetValue, columnWidth);
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

		DrawFloatControl("R", "##R", values.r, resetValue, columnWidth);
		ImGui::SameLine();
		DrawFloatControl("G", "##G", values.g, resetValue, columnWidth);
		ImGui::SameLine();
		DrawFloatControl("B", "##B", values.b, resetValue, columnWidth);
		ImGui::SameLine();
		DrawFloatControl("A", "##A", values.a, resetValue, columnWidth);


		ImGui::PopStyleColor(3);
		ImGui::PopStyleVar();
		EndColumns();
	}
}