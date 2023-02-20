#include "stdafx.h"
#include "Project.h"

#include "ProjectSerializer.h"
#include "XYZ/Plugin/PluginManager.h"
#include "XYZ/Script/ScriptEngine.h"

namespace XYZ {
	static Ref<Project> s_ActiveProject;

	std::string Plugin::BinaryPath() const
	{
		std::string result = PluginDirectory.string() + "/bin";
		result += "/" + std::string(XYZ_BINARY_DIR);
		result += "/" + PluginName;

#ifdef XYZ_PLATFORM_WINDOWS
		result += ".dll";
#elif XYZ_PLATFORM_UNIX
		result += ".so";
#endif
		return result;
	}

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
		for (const auto& plugin : s_ActiveProject->Configuration.Plugins)
		{
			if (plugin.Language == PluginLanguage::CPP)
			{
				PluginManager::ClosePlugin(plugin.BinaryPath());
			}
		}

		s_ActiveProject = ProjectSerializer::Deserialize(path);
		for (const auto& plugin : s_ActiveProject->Configuration.Plugins)
		{
			if (plugin.Language == PluginLanguage::CPP)
			{
				PluginManager::OpenPlugin(plugin.BinaryPath());
			}
			else if (plugin.Language == PluginLanguage::CS)
			{
				ScriptEngine::LoadRuntimeAssembly(plugin.BinaryPath());
			}
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
		for (const auto& plugin : s_ActiveProject->Configuration.Plugins)
		{
			if (plugin.Language == PluginLanguage::CPP)
			{
				PluginManager::ClosePlugin(plugin.BinaryPath());
			}
		}
		for (const auto& plugin : s_ActiveProject->Configuration.Plugins)
		{
			if (plugin.Language == PluginLanguage::CPP)
			{
				PluginManager::OpenPlugin(plugin.BinaryPath());
			}
			else if (plugin.Language == PluginLanguage::CS)
			{
				ScriptEngine::LoadRuntimeAssembly(plugin.BinaryPath());
			}
		}
	}

}