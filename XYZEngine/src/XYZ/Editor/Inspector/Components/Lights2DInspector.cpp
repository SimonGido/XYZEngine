#include "stdafx.h"
#include "Lights2DInspector.h"

#include "XYZ/Editor/EditorHelper.h"
#include "XYZ/Scene/Components.h"

#include "XYZ/ImGui/ImGui.h"

namespace XYZ {

	bool PointLight2DInspector::OnEditorRender(Ref<Renderer2D> renderer)
	{
		return EditorHelper::DrawComponent<PointLight2D>("Point Light2D", m_Context, [&](auto& component) {
			/*
			EditorHelper::DrawVec3ControlRGB("Color", component.Color);
			EditorHelper::BeginColumns("Radius");
			ImGui::DragFloat("##Radius", &component.Radius, 0.05f, 0.0f, 0.0f, "%.2f");
			EditorHelper::EndColumns();

			EditorHelper::BeginColumns("Intensity");
			ImGui::DragFloat("##Intensity", &component.Intensity, 0.05f, 0.0f, 0.0f, "%.2f");
			EditorHelper::EndColumns();
			*/
		});
	}
	bool SpotLight2DInspector::OnEditorRender(Ref<Renderer2D> renderer)
	{
		return EditorHelper::DrawComponent<SpotLight2D>("Spot Light2D", m_Context, [&](auto& component) {

			UI::ScopedStyleStack style(ImGuiStyleVar_ItemSpacing, ImVec2{ 0.0f, 5.0f });
			UI::ScopedColorStack color(
				ImGuiCol_Button, ImVec4{ 0.5f, 0.5f, 0.5f, 1.0f },
				ImGuiCol_ButtonHovered, ImVec4{ 0.6f, 0.6f, 0.6f, 1.0f },
				ImGuiCol_ButtonActive, ImVec4{ 0.65f, 0.65f, 0.65f, 1.0f }
			);
			const float lineHeight = GImGui->Font->FontSize + GImGui->Style.FramePadding.y * 2.0f;

			if (ImGui::BeginTable("##SpotLight2DTable", 2))
			{
				UI::TableRow("Color",
					[]() {ImGui::Text("Color"); },
					[&]() { UI::ScopedTableColumnAutoWidth scoped(3, lineHeight);
						   UI::Vec3Control({ "R", "G", "B" }, component.Color); }
				);

				UI::TableRow( "Radius",
					[]() { ImGui::Text("Radius"); },
					[&]() { UI::FloatControl("##Radius", UI::Utils::GenerateID(), component.Radius, 1.0f, 0.05f); }
				);
				
				UI::TableRow( "Intensity",
					[]() { ImGui::Text("Intensity"); },
					[&]() { UI::FloatControl("##Intensity", UI::Utils::GenerateID(), component.Intensity, 1.0f, 0.05f); }
				);

				UI::TableRow("InnerAngle",
					[]() { ImGui::Text("Inner Angle"); },
					[&]() { UI::FloatControl("##InnerAngle", UI::Utils::GenerateID(), component.InnerAngle, 0.0f, 0.05f); }
				);

				UI::TableRow("OuterAngle",
					[]() { ImGui::Text("OuterAngle"); },
					[&]() { UI::FloatControl("##OuterAngle", UI::Utils::GenerateID(), component.OuterAngle, 0.0f, 0.05f); }
				);
				ImGui::EndTable();
			}
		});
	}
}