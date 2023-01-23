#include "stdafx.h"
#include "Project.h"

#include "ProjectSerializer.h"
#include "XYZ/Plugin/PluginManager.h"

namespace XYZ {
	static Ref<Project> s_ActiveProject;

	Ref<Project> Project::GetActive()
	{
		return s_ActiveProject;
	}
	Ref<Project> Project::New()
	{
		s_ActiveProject = Ref<Project>::Create();
		return s_ActiveProject;
	}
	Ref<Project> Project::Load(const std::filesystem::path& path)
	{
		// Close old plugins
		for (const auto& pluginPath : s_ActiveProject->Configuration.PluginPaths)
		{
			PluginManager::ClosePlugin(pluginPath.string());
		}

		s_ActiveProject = ProjectSerializer::Deserialize(path);
		for (const auto& pluginPath : s_ActiveProject->Configuration.PluginPaths)
		{
			PluginManager::OpenPlugin(pluginPath.string());
		}
		return s_ActiveProject;
	}
	bool Project::SaveActive(const std::filesystem::path& path)
	{
		ProjectSerializer::Serialize(path, s_ActiveProject);
		s_ActiveProject->m_ProjectDirectory = path.parent_path();
		return true;
	}
	void Project::ReloadPlugins()
	{
		for (const auto& pluginPath : Configuration.PluginPaths)
		{
			PluginManager::ClosePlugin(pluginPath.string());
		}
		for (const auto& pluginPath : Configuration.PluginPaths)
		{
			PluginManager::OpenPlugin(pluginPath.string());
		}
	}
}