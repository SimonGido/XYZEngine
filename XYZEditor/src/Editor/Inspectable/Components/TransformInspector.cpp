#include "stdafx.h"
#include "TransformInspector.h"

#include "Editor/EditorHelper.h"
#include "XYZ/Scene/Components.h"

#include "XYZ/ImGui/ImGui.h"

namespace XYZ {
	namespace Editor {
		TransformInspector::TransformInspector()
			:
			Inspectable("TransformInspector")
		{
		}
		bool TransformInspector::OnEditorRender()
		{
			return EditorHelper::DrawComponent<TransformComponent>("Transform", m_Context, [&](auto& component) {

				if (ImGui::BeginTable("##TransformTable", 2, ImGuiTableFlags_SizingStretchProp))
				{
					UI::ScopedStyleStack style(true, ImGuiStyleVar_ItemSpacing, ImVec2{ 0.0f, 5.0f });
					UI::ScopedColorStack color(true,
						ImGuiCol_Button, ImVec4{ 0.5f, 0.5f, 0.5f, 1.0f },
						ImGuiCol_ButtonHovered, ImVec4{ 0.6f, 0.6f, 0.6f, 1.0f },
						ImGuiCol_ButtonActive, ImVec4{ 0.65f, 0.65f, 0.65f, 1.0f }
					);
					const float lineHeight = GImGui->Font->FontSize + GImGui->Style.FramePadding.y * GImGui->Font->Scale * 2.0f;
					glm::vec3 translation = component->Translation;
					glm::vec3 rotation = glm::degrees(component->Rotation);
					glm::vec3 scale = component->Scale;

					bool translationChanged = false, rotationChanged = false, scaleChanged = false;
					UI::TableRow("Translation",
						[]() { ImGui::Text("Translation"); },
						[&]() { UI::ScopedTableColumnAutoWidth scoped(3, lineHeight);
						translationChanged = UI::Vec3Control({ "X", "Y", "Z" }, translation); }
					);

					UI::TableRow("Rotation",
						[]() { ImGui::Text("Rotation"); },
						[&]() { UI::ScopedTableColumnAutoWidth scoped(3, lineHeight);
						rotationChanged = UI::Vec3Control({ "X", "Y", "Z" }, rotation); }
					);
					
					UI::TableRow("Scale",
						[]() { ImGui::Text("Scale"); },
						[&]() { UI::ScopedTableColumnAutoWidth scoped(3, lineHeight);
						scaleChanged = UI::Vec3Control({ "X", "Y", "Z" }, scale, 1.0f); }
					);

					if (translationChanged)
						component.GetTransform().Translation = translation;
					if (rotationChanged)
						component.GetTransform().Rotation = glm::radians(rotation);
					if (scaleChanged)
						component.GetTransform().Scale = scale;

					ImGui::EndTable();
				}
			});
		}
		void TransformInspector::SetSceneEntity(const SceneEntity& entity)
		{
			m_Context = entity;
		}
	}
}
