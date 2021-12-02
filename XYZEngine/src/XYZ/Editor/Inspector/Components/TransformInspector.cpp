#include "stdafx.h"
#include "TransformInspector.h"

#include "XYZ/Editor/EditorHelper.h"
#include "XYZ/Scene/Components.h"

#include "XYZ/ImGui/ImGui.h"

namespace XYZ {

	bool TransformInspector::OnEditorRender(Ref<Renderer2D> renderer)
	{
		return EditorHelper::DrawComponent<TransformComponent>("Transform", m_Context, [&](auto& component) {

			if (ImGui::BeginTable("##TransformTable", 2, ImGuiTableFlags_SizingFixedFit))
			{
				UI::ScopedStyleStack style(ImGuiStyleVar_ItemSpacing, ImVec2{ 0.0f, 5.0f });
				UI::ScopedColorStack color(
					ImGuiCol_Button, ImVec4{ 0.5f, 0.5f, 0.5f, 1.0f },
					ImGuiCol_ButtonHovered, ImVec4{ 0.6f, 0.6f, 0.6f, 1.0f },
					ImGuiCol_ButtonActive, ImVec4{ 0.65f, 0.65f, 0.65f, 1.0f }
				);

				UI::TableRow(
					[]()  { ImGui::Text("Translation"); },
					[&]() { EditorHelper::DrawVec3Control("Translation", component.Translation); }
				);	
				glm::vec3 rotation = glm::degrees(component.Rotation);
				UI::TableRow(
					[]() { ImGui::Text("Rotation");},
					[&]() {EditorHelper::DrawVec3Control("Rotation", rotation); }
				);
				component.Rotation = glm::radians(rotation);
			
				UI::TableRow(
					[]() { ImGui::Text("Scale");},
					[&]() {EditorHelper::DrawVec3Control("Scale", component.Scale, 1.0f); }
				);
				ImGui::EndTable();
			}
		});
	}
}
