#include "stdafx.h"
#include "Lights2DInspector.h"

#include "Editor/EditorHelper.h"
#include "XYZ/Scene/Components.h"

#include "XYZ/ImGui/ImGui.h"

namespace XYZ {
	namespace Editor {
		SpotLight2DInspector::SpotLight2DInspector()
			:
			Inspectable("SpotLight2DInspector")
		{
		}
		bool SpotLight2DInspector::OnEditorRender()
		{
			return EditorHelper::DrawComponent<SpotLight2D>("Spot Light2D", m_Context, [&](auto& component) {

				UI::ScopedStyleStack style(true, ImGuiStyleVar_ItemSpacing, ImVec2{ 0.0f, 5.0f });
				UI::ScopedColorStack color(true,
					ImGuiCol_Button, ImVec4{ 0.5f, 0.5f, 0.5f, 1.0f },
					ImGuiCol_ButtonHovered, ImVec4{ 0.6f, 0.6f, 0.6f, 1.0f },
					ImGuiCol_ButtonActive, ImVec4{ 0.65f, 0.65f, 0.65f, 1.0f }
				);
				const float lineHeight = GImGui->Font->FontSize + GImGui->Style.FramePadding.y * 2.0f;

				if (ImGui::BeginTable("##SpotLight2DTable", 2, ImGuiTableFlags_SizingStretchProp))
				{
					UI::TableRow("Color",
						[]() {ImGui::Text("Color"); },
						[&]() { UI::ScopedTableColumnAutoWidth scoped(3, lineHeight);
					UI::Vec3Control({ "R", "G", "B" }, component.Color); }
					);

					UI::TableRow("Radius",
						[]() { ImGui::Text("Radius"); },
						[&]() { UI::FloatControl("##Radius", "##RadiusDrag", component.Radius, 1.0f, 0.05f); }
					);

					UI::TableRow("Intensity",
						[]() { ImGui::Text("Intensity"); },
						[&]() { UI::FloatControl("##Intensity", "##IntensityDrag", component.Intensity, 1.0f, 0.05f); }
					);

					UI::TableRow("InnerAngle",
						[]() { ImGui::Text("Inner Angle"); },
						[&]() { UI::FloatControl("##InnerAngle", "##InnerAngleDrag", component.InnerAngle, 0.0f, 0.05f); }
					);

					UI::TableRow("OuterAngle",
						[]() { ImGui::Text("OuterAngle"); },
						[&]() { UI::FloatControl("##OuterAngle", "##OuterAngleDrag", component.OuterAngle, 0.0f, 0.05f); }
					);
					ImGui::EndTable();
				}
			});
		}
		void SpotLight2DInspector::SetSceneEntity(const SceneEntity& entity)
		{
			m_Context = entity;
		}
		PointLight2DInspector::PointLight2DInspector()
			:
			Inspectable("PointLight2DInspector")
		{
		}
		bool PointLight2DInspector::OnEditorRender()
		{
			return EditorHelper::DrawComponent<PointLight2D>("Point Light2D", m_Context, [&](auto& component) {

				UI::ScopedStyleStack style(true, ImGuiStyleVar_ItemSpacing, ImVec2{ 0.0f, 5.0f });
				UI::ScopedColorStack color(true,
					ImGuiCol_Button, ImVec4{ 0.5f, 0.5f, 0.5f, 1.0f },
					ImGuiCol_ButtonHovered, ImVec4{ 0.6f, 0.6f, 0.6f, 1.0f },
					ImGuiCol_ButtonActive, ImVec4{ 0.65f, 0.65f, 0.65f, 1.0f }
				);
				const float lineHeight = GImGui->Font->FontSize + GImGui->Style.FramePadding.y * 2.0f;

				if (ImGui::BeginTable("##SpotLight2DTable", 2, ImGuiTableFlags_SizingStretchProp))
				{
					UI::TableRow("Color",
						[]() {ImGui::Text("Color"); },
						[&]() { UI::ScopedTableColumnAutoWidth scoped(3, lineHeight);
					UI::Vec3Control({ "R", "G", "B" }, component.Color); }
					);

					UI::TableRow("Radius",
						[]() { ImGui::Text("Radius"); },
						[&]() { UI::FloatControl("##Radius", "##RadiusDrag", component.Radius, 1.0f, 0.05f); }
					);

					UI::TableRow("Intensity",
						[]() { ImGui::Text("Intensity"); },
						[&]() { UI::FloatControl("##Intensity", "##IntensityDrag", component.Intensity, 1.0f, 0.05f); }
					);

					ImGui::EndTable();
				}
			});
		}
		void PointLight2DInspector::SetSceneEntity(const SceneEntity& entity)
		{
			m_Context = entity;
		}
	}
}