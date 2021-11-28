#include "stdafx.h"
#include "Lights2DInspector.h"

#include "XYZ/Editor/EditorHelper.h"
#include "XYZ/Scene/Components.h"


namespace XYZ {

	bool PointLight2DInspector::OnEditorRender(Ref<Renderer2D> renderer)
	{
		return EditorHelper::DrawComponent<PointLight2D>("Point Light2D", m_Context, [&](auto& component) {

			EditorHelper::DrawVec3ControlRGB("Color", component.Color);
			EditorHelper::BeginColumns("Radius");
			ImGui::DragFloat("##Radius", &component.Radius, 0.05f, 0.0f, 0.0f, "%.2f");
			EditorHelper::EndColumns();

			EditorHelper::BeginColumns("Intensity");
			ImGui::DragFloat("##Intensity", &component.Intensity, 0.05f, 0.0f, 0.0f, "%.2f");
			EditorHelper::EndColumns();
		});
	}
	bool SpotLight2DInspector::OnEditorRender(Ref<Renderer2D> renderer)
	{
		return EditorHelper::DrawComponent<SpotLight2D>("Spot Light2D", m_Context, [&](auto& component) {

			EditorHelper::DrawVec3ControlRGB("Color", component.Color);
			EditorHelper::BeginColumns("Radius");
			ImGui::DragFloat("##Radius", &component.Radius, 0.05f, 0.0f, 0.0f, "%.2f");
			EditorHelper::EndColumns();

			EditorHelper::BeginColumns("Intensity");
			ImGui::DragFloat("##Intensity", &component.Intensity, 0.05f, 0.0f, 0.0f, "%.2f");
			EditorHelper::EndColumns();

			EditorHelper::BeginColumns("Inner Angle");
			ImGui::DragFloat("##InnerAngle", &component.InnerAngle, 1.0f, 0.0f, 0.0f, "%.2f");
			EditorHelper::EndColumns();

			EditorHelper::BeginColumns("Outer Angle");
			ImGui::DragFloat("##OuterAngle", &component.OuterAngle, 1.0f, 0.0f, 0.0f, "%.2f");
			EditorHelper::EndColumns();
		});
	}
}