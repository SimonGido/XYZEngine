#include "stdafx.h"
#include "EditorConsolePanel.h"

#include <imgui.h>

namespace XYZ {
	namespace Editor {
		EditorConsolePanel::EditorConsolePanel(std::string name)
			:
			EditorPanel(std::move(name))
		{
		}
		void EditorConsolePanel::OnImGuiRender(bool& open)
		{
			if (ImGui::Begin("Console", &open))
			{
				for (const auto& message : m_Messages)
				{
					if (message.MessageCategory == ConsoleMessage::Category::Info)
					{
						ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(0.3f, 1.0f, 0.3f, 1.0f));
					}
					else if (message.MessageCategory == ConsoleMessage::Category::Warning)
					{
						ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(0.8f, 1.0f, 0.3f, 1.0f));
					}
					else
					{
						ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(1.0f, 0.2f, 0.3f, 1.0f));
					}
					ImGui::TextUnformatted(message.Message.c_str());
					ImGui::PopStyleColor();
					if (!ImGui::IsWindowFocused())
						ImGui::SetScrollHereY(1.0f);
				}
			}
			ImGui::End();
		}
		void EditorConsolePanel::OnUpdate(Timestep ts)
		{
		}
		bool EditorConsolePanel::OnEvent(Event& e)
		{
			return false;
		}
		void EditorConsolePanel::SetSceneContext(const Ref<Scene>& scene)
		{
		}
	}
}