#include "stdafx.h"
#include "EditorManager.h"

#include "XYZ/Core/Application.h"

#include <imgui.h>

namespace XYZ {
	namespace Editor {
		EditorPanel::EditorPanel(std::string name)
			:
			m_Name(std::move(name))
		{
		}
		void EditorManager::OnImGuiRender()
		{
			displayMenuBar();
			for (auto& view : m_EditorPanels)
			{
				if (view.Open)
				{
					view.Panel->OnImGuiRender(view.Open);
				}
			}
		}
		void EditorManager::OnUpdate(Timestep ts)
		{
			for (auto& view : m_EditorPanels)
				view.Panel->OnUpdate(ts);
		}
		bool EditorManager::OnEvent(Event& e)
		{
			for (auto& view : m_EditorPanels)
			{
				if (view.Panel->OnEvent(e))
				{
					e.Handled = true;
					return true;
				}
			}
			return false;
		}
		void EditorManager::SetSceneContext(const Ref<Scene>& scene)
		{
			m_Scene = scene;
			for (auto& panel : m_EditorPanels)
			{
				panel.Panel->SetSceneContext(scene);
			}
		}
		Ref<EditorPanel> EditorManager::getPanel(const std::string& name) const
		{
			for (auto& panel : m_EditorPanels)
			{
				if (panel.Panel->GetName() == name)
					return panel.Panel;
			}
			return nullptr;
		}
		void EditorManager::displayMenuBar()
		{
			if (ImGui::BeginMenuBar())
			{
				if (ImGui::BeginMenu("File"))
				{
					if (ImGui::MenuItem("Open...", "Ctrl+O"))
					{}

					if (ImGui::MenuItem("Save As...", "Ctrl+Shift+S"))
					{}
					if (ImGui::MenuItem("Exit"))
					{
						Application::Get().Stop();
					}
					ImGui::EndMenu();
				}
				if (ImGui::BeginMenu("View"))
				{
					for (auto& view : m_EditorPanels)
					{
						ImGui::MenuItem(view.Panel->GetName().c_str(), nullptr, &view.Open);
					}
					ImGui::EndMenu();
				}
				ImGui::EndMenuBar();
			}
		}
	}
}