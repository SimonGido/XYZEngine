#include "stdafx.h"	
#include "ParticleComponentGPUInspector.h"


#include "Editor/EditorHelper.h"
#include "XYZ/Scene/Components.h"

#include "XYZ/ImGui/ImGui.h"

namespace XYZ {
	namespace Editor {
		ParticleComponentGPUInspector::ParticleComponentGPUInspector()
			:
			Inspectable("ParticleComponentGPUInspector")
		{
		}

		bool ParticleComponentGPUInspector::OnEditorRender()
		{
			return EditorHelper::DrawComponent<ParticleComponentGPU>("Particle Component GPU", m_Context, [&](auto& component) {

				if (ImGui::BeginTable("##TransformTable", 2, ImGuiTableFlags_SizingStretchProp))
				{
					UI::ScopedStyleStack style(true, ImGuiStyleVar_ItemSpacing, ImVec2{ 0.0f, 5.0f });
					UI::ScopedColorStack color(true,
						ImGuiCol_Button, ImVec4{ 0.5f, 0.5f, 0.5f, 1.0f },
						ImGuiCol_ButtonHovered, ImVec4{ 0.6f, 0.6f, 0.6f, 1.0f },
						ImGuiCol_ButtonActive, ImVec4{ 0.65f, 0.65f, 0.65f, 1.0f }
					);
					const float lineHeight = GImGui->Font->FontSize + GImGui->Style.FramePadding.y * GImGui->Font->Scale * 2.0f;

					UI::TableRow("Translation",
						[]() { ImGui::Text("Speed"); },
						[&]() { UI::FloatControl("##Speed", "##SpeedDrag", component.System->Speed, 1.0f, 0.1f); }
					);

					UI::TableRow("Loop",
						[]() { ImGui::Text("Loop"); },
						[&]() { ImGui::Checkbox("##Loop", &component.System->Loop); }
					);


					UI::TableRow("EmittedParticles",
						[&]() { ImGui::Text("Emitted Particles %u", component.System->GetEmittedParticles()); },
						[&]() {  }
					);
					ImGui::EndTable();
				}
				});
		}

		void ParticleComponentGPUInspector::SetSceneEntity(const SceneEntity& entity)
		{
			m_Context = entity;
		}
	}
}