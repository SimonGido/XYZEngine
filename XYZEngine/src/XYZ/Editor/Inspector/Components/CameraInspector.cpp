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
				
				ImGui::InputFloat("##Size", &props.OrthographicSize);


				ImGui::InputFloat("##Near", &props.OrthographicNear);

				ImGui::InputFloat("##Far", &props.OrthographicFar);
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
		});
	}
}