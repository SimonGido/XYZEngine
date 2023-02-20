#include "stdafx.h"
#include "ProjectSerializer.h"

#include <fstream>
#include <yaml-cpp/yaml.h>

namespace XYZ {
	void ProjectSerializer::Serialize(const std::filesystem::path& filepath, Ref<Project> project)
	{
		const auto& config = project->Configuration;

		YAML::Emitter out;
		{
			out << YAML::BeginMap; // Root
			out << YAML::Key << "Project" << YAML::Value;
			{
				out << YAML::BeginMap;// Project
				out << YAML::Key << "Name" << YAML::Value << config.Name;
				out << YAML::Key << "StartScene" << YAML::Value << config.StartScene.string();
				out << YAML::Key << "AssetDirectory" << YAML::Value << config.AssetDirectory.string();
				
				out << YAML::Key << "Flags" << YAML::Value << config.Flags;
				out << YAML::Key << "Plugins" << YAML::BeginSeq << YAML::Flow; // Plugins
				for (const auto& plugin : project->Configuration.Plugins)
				{
					out << plugin.PluginDirectory.string();
				}
				out << YAML::EndSeq; // Plugins
				out << YAML::EndMap; // Project
			}
			out << YAML::EndMap; // Root
		}

		std::ofstream fout(filepath);
		fout << out.c_str();

	}
	Ref<Project> ProjectSerializer::Deserialize(const std::filesystem::path& filepath)
	{
		std::ifstream stream(filepath);
		std::stringstream strStream;
		strStream << stream.rdbuf();
		YAML::Node data = YAML::Load(strStream.str());

		YAML::Node projectData = data["Project"];

		Ref<Project> result = Ref<Project>::Create();
		result->Configuration.Name = projectData["Name"].as<std::string>();
		result->Configuration.StartScene = projectData["StartScene"].as<std::string>();
		result->Configuration.AssetDirectory = projectData["AssetDirectory"].as<std::string>();
		result->Configuration.Flags = projectData["Flags"].as<uint32_t>();
		YAML::Node pluginsData = data["Plugins"];
		for (auto it : data)
		{
			Plugin plugin;
			plugin.PluginDirectory = it.as<std::string>();
			result->Configuration.Plugins.push_back(plugin);
		}
		return result;
	}
}
