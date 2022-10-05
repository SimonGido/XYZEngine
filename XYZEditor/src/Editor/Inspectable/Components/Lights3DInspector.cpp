#include "stdafx.h"
#include "Lights3DInspector.h"

#include "Editor/EditorHelper.h"
#include "XYZ/Scene/Components.h"

#include "XYZ/ImGui/ImGui.h"

namespace XYZ {
	namespace Editor {
		PointLightComponent3DInspector::PointLightComponent3DInspector()
			:
			Inspectable("PointLightComponent3DInspector")
		{
		}
		bool PointLightComponent3DInspector::OnEditorRender()
		{
			return EditorHelper::DrawComponent<PointLightComponent3D>("Point Light3D", m_Context, [&](auto& component) {

				UI::ScopedStyleStack style(true, ImGuiStyleVar_ItemSpacing, ImVec2{ 0.0f, 5.0f });
				UI::ScopedColorStack color(true,
					ImGuiCol_Button, ImVec4{ 0.5f, 0.5f, 0.5f, 1.0f },
					ImGuiCol_ButtonHovered, ImVec4{ 0.6f, 0.6f, 0.6f, 1.0f },
					ImGuiCol_ButtonActive, ImVec4{ 0.65f, 0.65f, 0.65f, 1.0f }
				);
				const float lineHeight = GImGui->Font->FontSize + GImGui->Style.FramePadding.y * 2.0f;

				if (ImGui::BeginTable("##PointLight3DTable", 2, ImGuiTableFlags_SizingStretchProp))
				{
					UI::TableRow("Color",
						[]() {ImGui::Text("Radiance"); },
						[&]() { ImGui::ColorEdit3("##Radiance", glm::value_ptr(component.Radiance)); }
					);

					UI::TableRow("Intensity",
						[]() { ImGui::Text("Intensity"); },
						[&]() { UI::FloatControl("##Intensity", "##IntensityDrag", component.Intensity, 1.0f, 0.05f); }
					);

					UI::TableRow("LightSize",
						[]() { ImGui::Text("LightSize"); },
						[&]() { UI::FloatControl("##LightSize", "##LightSizeDrag", component.LightSize, 1.0f, 0.05f); }
					);

					UI::TableRow("MinRadius",
						[]() { ImGui::Text("MinRadius"); },
						[&]() { UI::FloatControl("##MinRadius", "##MinRadiusDrag", component.MinRadius, 0.0f, 0.05f); }
					);

					UI::TableRow("Radius",
						[]() { ImGui::Text("Radius"); },
						[&]() { UI::FloatControl("##Radius", "##RadiusDrag", component.Radius, 0.0f, 0.05f); }
					);
					UI::TableRow("CastsShadows",
						[]() { ImGui::Text("CastsShadows"); },
						[&]() { ImGui::Checkbox("##CastsShadows", &component.CastsShadows); }
					);
					UI::TableRow("SoftShadows",
						[]() { ImGui::Text("SoftShadows"); },
						[&]() { ImGui::Checkbox("##SoftShadows", &component.SoftShadows); }
					);
					UI::TableRow("Falloff",
						[]() { ImGui::Text("Falloff"); },
						[&]() { UI::FloatControl("##Falloff", "##FalloffDrag", component.Falloff, 0.0f, 0.05f); }
					);
					ImGui::EndTable();
				}
				});
		}
	}
}