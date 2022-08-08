#include "stdafx.h"
#include "CameraInspector.h"

#include "Editor/EditorHelper.h"
#include "XYZ/Scene/Components.h"

#include "XYZ/ImGui/ImGui.h"

namespace XYZ {
	namespace Editor {
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

					UI::ScopedStyleStack style(true, ImGuiStyleVar_ItemSpacing, ImVec2{ 0.0f, 5.0f });
					UI::ScopedColorStack color(true,
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
						CameraPerspectiveProperties props = camera.GetPerspectiveProperties();

						UI::TableRow("FOV",
							[]() {ImGui::Text("FOV"); },
							[&]() { UI::FloatControl("##FOV", "##FovDrag", props.PerspectiveFOV, 1.0f, 0.05f); }
						);
						UI::TableRow("Near",
							[]() {ImGui::Text("Near"); },
							[&]() { UI::FloatControl("##Near", "##NearDrag", props.PerspectiveNear, 0.0f, 0.05f); }
						);
						UI::TableRow("Far",
							[]() {ImGui::Text("Far"); },
							[&]() { UI::FloatControl("##Far", "##FarDrag", props.PerspectiveFar, 1.0f, 0.05f); }
						);

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
}