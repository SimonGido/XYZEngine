#include "stdafx.h"
#include "ScriptPanel.h"

#include "XYZ/Script/ScriptEngine.h"

#include "XYZ/ImGui/ImGui.h"

namespace XYZ {
	namespace Editor {
		ScriptPanel::ScriptPanel(std::string name)
			:
			EditorPanel(std::move(name))
		{
		}
		void ScriptPanel::OnImGuiRender(bool& open)
		{
			if (ImGui::Begin("Script Panel", &open))
			{
				Ref<Scene> scene = ScriptEngine::GetCurrentSceneContext();
				if (scene->GetState() == SceneState::Play)
				{
					ImGui::PushItemFlag(ImGuiItemFlags_Disabled, true);
					ImGui::PushStyleVar(ImGuiStyleVar_Alpha, ImGui::GetStyle().Alpha * 0.5f);
				}

				if (ImGui::Button("Reload"))
				{
					ScriptEngine::LoadRuntimeAssembly("Assets/Scripts/XYZScript.dll");
				}


				if (scene->GetState() == SceneState::Play)
				{
					ImGui::PopItemFlag();
					ImGui::PopStyleVar();
				}

			}
			ImGui::End();
		}
	}
}