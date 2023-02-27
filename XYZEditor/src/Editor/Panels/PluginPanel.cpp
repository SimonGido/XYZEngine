#include "stdafx.h"
#include "PluginPanel.h"



#include <imgui/imgui.h>

namespace XYZ {
	namespace Editor {

		struct BuildClassMember
		{
			BuildClassMember(const std::string& type, const std::string& name, const std::string& constructValue)
				:
				Type(type), Name(name), ConstructValue(constructValue)
			{
			}

			std::string Type;
			std::string Name;
			std::string ConstructValue;
		};

		struct BuildClassDefinition
		{
			std::string Name;
			
			std::vector<BuildClassMember> Members;
		};


		static std::string GenerateBuildClass(const BuildClassDefinition& definition)
		{
			std::string result = fmt::format("public class {}\n", definition.Name);
			result += "{\n"; // Class start bracket
				
			for (const auto& member : definition.Members)
			{
				result += fmt::format("\tpublic {} {}; \n", member.Type, member.Name);
			}

			result += fmt::format("\tpublic {}(string engineDirectory, string engineSourceDirectory)\n", definition.Name);
			result += "\t{\n"; // Constructor start bracket

			for (const auto& member : definition.Members)
			{
				result += fmt::format("\t\t{} = {};\n", member.Name, member.ConstructValue);
			}

			result += "\t}\n"; // Constructor end bracket
			result += "}"; // Class end bracket

			return result;
		}


		void PluginLanguagePopup(PluginLanguage& language, std::string& languageStr)
		{
			ImGui::InputText("##PluginLanguage", (char*)languageStr.c_str(), languageStr.size(), ImGuiInputTextFlags_ReadOnly);

			if (UI::IsTextActivated())
			{
				ImGui::OpenPopup("Plugin Language");
			}
			if (ImGui::BeginPopup("Plugin Language"))
			{
				if (ImGui::MenuItem("C++"))
				{
					language = PluginLanguage::CPP;
					languageStr = Plugin::PluginLanguageToString(language);
				}
				if (ImGui::MenuItem("C#"))
				{
					language = PluginLanguage::CS;
					languageStr = Plugin::PluginLanguageToString(language);
				}
				ImGui::EndPopup();
			}
		}

		PluginPanel::PluginPanel(std::string name) 
			:
			EditorPanel(std::move(name))
		{
			m_CreatePluginLanguage = PluginLanguage::CS;
			m_CreatePluginLanguageString = Plugin::PluginLanguageToString(m_CreatePluginLanguage);
		}
		void PluginPanel::OnImGuiRender(bool& open)
		{
			if (ImGui::Begin("Plugin Window", &open))
			{
				UI::InputText("Plugin Name", m_CreatePluginName);
				UI::InputText("Plugin Directory", m_CreatePluginDirectory, ImGuiInputTextFlags_ReadOnly);
			
				if (UI::IsTextActivated())
				{
					std::string appDir = Application::Get().GetApplicationDirectory().string();
					m_CreatePluginDirectory = FileSystem::OpenFolder(Application::Get().GetWindow().GetNativeWindow(), appDir);
				}

				PluginLanguagePopup(m_CreatePluginLanguage, m_CreatePluginLanguageString);

				bool canCreate = !m_CreatePluginDirectory.empty() && !m_CreatePluginName.empty();

				if (!canCreate)
					ImGui::BeginDisabled();
				
				if (ImGui::Button("Create Plugin"))
				{
					BuildClassDefinition definition;
					definition.Name = m_CreatePluginName;
					definition.Members.emplace_back("string", "ProjectName", fmt::format("\"{}\"", m_CreatePluginName));
					definition.Members.emplace_back("string", "Language", fmt::format("\"{}\"", m_CreatePluginLanguageString));


					std::string buildClass = GenerateBuildClass(definition);
					std::string buildFile = m_CreatePluginDirectory + "/" + m_CreatePluginName + ".Build.cs";

					FileSystem::WriteFile(buildFile, buildClass);

					const std::filesystem::path engineBinaryDir = std::filesystem::absolute(Application::Get().GetEngineBinaryDirectory());
					const std::filesystem::path engineSourceDir = std::filesystem::absolute(Application::Get().GetEngineSourceDirectory());

					std::filesystem::path command = engineBinaryDir.parent_path() / "XYZPluginGenerator/XYZPluginGenerator.exe";
					std::string args = fmt::format("{} {} {}", m_CreatePluginDirectory, engineBinaryDir, engineSourceDir);

					std::string result;
					if (Platform::ExecuteCommand(command.string().c_str(), args.data(), result))
					{
						XYZ_CORE_INFO(result);
						Ref<Project> project = Project::GetActive();
						project->Configuration.PluginPaths.push_back(m_CreatePluginDirectory);
						project->ReloadPlugins();
					}
					else
					{
						XYZ_CORE_WARN("Failed to create plugin");
					}
				}
				if (!canCreate)
					ImGui::EndDisabled();

				if (ImGui::Button("Load Plugin"))
				{
					std::string appDir = Application::Get().GetApplicationDirectory().string();
					std::string pluginDir = FileSystem::OpenFolder(Application::Get().GetWindow().GetNativeWindow(), appDir);
					
					Ref<Project> project = Project::GetActive();
					project->Configuration.PluginPaths.push_back(pluginDir);
					project->ReloadPlugins();
				}
			}
			ImGui::End();
		}
		void PluginPanel::OnUpdate(Timestep ts)
		{
		}
		bool PluginPanel::OnEvent(Event& e)
		{
			return false;
		}
		void PluginPanel::SetSceneContext(const Ref<Scene>& scene)
		{
		}
	}
}