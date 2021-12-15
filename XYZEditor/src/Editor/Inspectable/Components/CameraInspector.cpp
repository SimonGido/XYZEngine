#include "stdafx.h"
#include "CameraInspector.h"

#include "Editor/EditorHelper.h"
#include "XYZ/Scene/Components.h"

#include "XYZ/ImGui/ImGui.h"

namespace XYZ {

	CameraInspector::CameraInspector()
		:
		Inspectable("CameraInspector")
	{
	}
	bool CameraInspector::OnEditorRender()
	{
		return EditorHelper::DrawComponent<CameraComponent>("Camera", m_Context, [&](auto& component) {

			auto& camera = component.Camera;

			if (ImGui::BeginTable("##TransformTable", 2, ImGuiTableFlags_SizingStretchProp))
			{
				UI::TableRow("Projection",
					[&]() { 
					if (ImGui::Button("Projection")) 
						ImGui::OpenPopup("Projection Type"); 
					if (ImGui::BeginPopup("Projection Type"))
					{
						if (ImGui::MenuItem("Perspective"))
						{
							camera.SetProjectionType(CameraProjectionType::Perspective);
							ImGui::CloseCurrentPopup();
						}
						if (ImGui::MenuItem("Orthographic"))
						{
							camera.SetProjectionType(CameraProjectionType::Orthographic);
							ImGui::CloseCurrentPopup();
						}
						ImGui::EndPopup();
					}
					},
					[&]() { 
						const char* text = camera.GetProjectionType() == CameraProjectionType::Orthographic ? "Orthographic" : "Perspective";
						ImGui::Text(text);
					}
				);
				
				UI::ScopedStyleStack style(ImGuiStyleVar_ItemSpacing, ImVec2{ 0.0f, 5.0f });
				UI::ScopedColorStack color(
					ImGuiCol_Button, ImVec4{ 0.5f, 0.5f, 0.5f, 1.0f },
					ImGuiCol_ButtonHovered, ImVec4{ 0.6f, 0.6f, 0.6f, 1.0f },
					ImGuiCol_ButtonActive, ImVec4{ 0.65f, 0.65f, 0.65f, 1.0f }
				);
				if (camera.GetProjectionType() == CameraProjectionType::Orthographic)
				{
					CameraOrthographicProperties props = camera.GetOrthographicProperties();
					UI::TableRow("Size",
						[]() {ImGui::Text("Size"); },
						[&]() { UI::FloatControl("##Size", "##SizeDrag", props.OrthographicSize, 1.0f, 0.05f); }
					);
					UI::TableRow("Near",
						[]() {ImGui::Text("Near"); },
						[&]() { UI::FloatControl("##Near", "##NearDrag", props.OrthographicNear, 0.0f, 0.05f); }
					);
					UI::TableRow("Far",
						[]() {ImGui::Text("Far"); },
						[&]() { UI::FloatControl("##Far", "##FarDrag", props.OrthographicFar, 1.0f, 0.05f); }
					);
					camera.SetOrthographic(props);
				}
				else
				{
					ImGui::Text("Perspective");
					CameraPerspectiveProperties props = camera.GetPerspectiveProperties();
					ImGui::InputFloat("##FOV", &props.PerspectiveFOV);

					ImGui::InputFloat("##Near", &props.PerspectiveNear);

					ImGui::InputFloat("##Far", &props.PerspectiveFar);
					camera.SetPerspective(props);
				}
				ImGui::EndTable();
			}
		});
	}
	void CameraInspector::SetSceneEntity(const SceneEntity& entity)
	{
		m_Context = entity;
	}
}