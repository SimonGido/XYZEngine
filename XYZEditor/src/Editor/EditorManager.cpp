#include "stdafx.h"
#include "EditorManager.h"

#include "XYZ/Core/Application.h"

#include "XYZ/Scene/SceneSerializer.h"
#include "XYZ/Project/ProjectSerializer.h"

#include "XYZ/Utils/FileSystem.h"

#include "Editor/Event/EditorEvents.h"

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
			if (e.GetEventType() == EventType::Editor)
			{
				EditorEvent& editorEvent = (EditorEvent&)e;
				if (editorEvent.GetEditorEventType() == EditorEventType::SceneLoaded)
				{
					SceneLoadedEvent& sceneLoadedEvent = (SceneLoadedEvent&)editorEvent;
					for (auto& view : m_EditorPanels)
					{
						view.Panel->SetSceneContext(sceneLoadedEvent.GetScene());
					}
					return true;
				}
			}

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
		void EditorManager::Clear()
		{
			m_EditorPanels.clear();
		}
		void EditorManager::SetSceneContext(const Ref<Scene>& scene)
		{
			m_Scene = scene;
			for (auto& panel : m_EditorPanels)
			{
				panel.Panel->SetSceneContext(scene);
			}
		}

		std::shared_ptr<EditorPanel> EditorManager::getPanel(const std::string& name) const
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
					{
						
					}

					if (ImGui::MenuItem("Save As...", "Ctrl+Shift+S"))
					{
						std::string filepath = FileSystem::OpenFile(Application::Get().GetWindow().GetNativeWindow(), ".xyzproj");
						std::filesystem::path projectPath(filepath);
						Project::SaveActive(projectPath);
					}

					if (ImGui::MenuItem("New Project..."))
					{
						std::string directory = FileSystem::OpenFolder(Application::Get().GetWindow().GetNativeWindow(), "");
						Project::New();

					}
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