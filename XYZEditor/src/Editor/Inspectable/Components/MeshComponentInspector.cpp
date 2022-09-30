#include "stdafx.h"
#include "MeshComponentInspector.h"


#include "Editor/EditorHelper.h"
#include "XYZ/Scene/Components.h"

#include "XYZ/ImGui/ImGui.h"

namespace XYZ {
	namespace Editor {
		MeshComponentInspector::MeshComponentInspector()
			:
			Inspectable("Mesh Inspector")
		{
		}
		bool MeshComponentInspector::OnEditorRender()
		{
			return EditorHelper::DrawComponent<MeshComponent>("Mesh Component", m_Context, [&](auto& component) {

			});
		}
		void MeshComponentInspector::SetSceneEntity(const SceneEntity& entity)
		{
			m_Context = entity;
		}
		AnimatedMeshComponentInspector::AnimatedMeshComponentInspector()
			:
			Inspectable("Animated Mesh Inspector")
		{
		}
		bool AnimatedMeshComponentInspector::OnEditorRender()
		{
			return EditorHelper::DrawComponent<AnimatedMeshComponent>("Animated Mesh Component", m_Context, [&](auto& component) {
				UI::ScopedStyleStack style(true, ImGuiStyleVar_ItemSpacing, ImVec2{ 0.0f, 5.0f });
				UI::ScopedColorStack color(true,
					ImGuiCol_Button, ImVec4{ 0.5f, 0.5f, 0.5f, 1.0f },
					ImGuiCol_ButtonHovered, ImVec4{ 0.6f, 0.6f, 0.6f, 1.0f },
					ImGuiCol_ButtonActive, ImVec4{ 0.65f, 0.65f, 0.65f, 1.0f }
				);
				const float lineHeight = GImGui->Font->FontSize + GImGui->Style.FramePadding.y * 2.0f;

				if (ImGui::BeginTable("##AnimatedMeshTable", 2, ImGuiTableFlags_SizingStretchProp))
				{

					ImGui::EndTable();
				}
			});
		}
		void AnimatedMeshComponentInspector::SetSceneEntity(const SceneEntity& entity)
		{
			m_Context = entity;
		}
	}
}