#include "stdafx.h"
#include "CameraInspector.h"

#include "XYZ/Editor/EditorHelper.h"
#include "XYZ/Scene/Components.h"

namespace XYZ {
	bool CameraInspector::OnEditorRender(Ref<Renderer2D> renderer)
	{
		return EditorHelper::DrawComponent<CameraComponent>("Camera", m_Context, [&](auto& component) {

			auto& camera = component.Camera;
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

			ImGui::SameLine();
			if (camera.GetProjectionType() == CameraProjectionType::Orthographic)
			{
				ImGui::Text("Orthographic");
				CameraOrthographicProperties props = camera.GetOrthographicProperties();
				EditorHelper::BeginColumns("Size");
				ImGui::InputFloat("##Size", &props.OrthographicSize);
				EditorHelper::EndColumns();

				EditorHelper::BeginColumns("Near");
				ImGui::InputFloat("##Near", &props.OrthographicNear);
				EditorHelper::EndColumns();

				EditorHelper::BeginColumns("Far");
				ImGui::InputFloat("##Far", &props.OrthographicFar);
				EditorHelper::EndColumns();
				camera.SetOrthographic(props);
			}
			else
			{
				ImGui::Text("Perspective");
				CameraPerspectiveProperties props = camera.GetPerspectiveProperties();
				EditorHelper::BeginColumns("FOV");
				ImGui::InputFloat("##FOV", &props.PerspectiveFOV);
				EditorHelper::EndColumns();

				EditorHelper::BeginColumns("Near");
				ImGui::InputFloat("##Near", &props.PerspectiveNear);
				EditorHelper::EndColumns();

				EditorHelper::BeginColumns("Far");
				ImGui::InputFloat("##Far", &props.PerspectiveFar);
				EditorHelper::EndColumns();
				camera.SetPerspective(props);
			}
		});
	}
}