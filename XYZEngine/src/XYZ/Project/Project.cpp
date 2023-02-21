#include "stdafx.h"
#include "Project.h"

#include "ProjectSerializer.h"
#include "XYZ/Plugin/PluginManager.h"
#include "XYZ/Script/ScriptEngine.h"

#include <yaml-cpp/yaml.h>

namespace XYZ {
	namespace Utils {

		enum class PluginLanguage
		{
			None,
			CPP,
			CS
		};

		std::string PluginLanguageToString(PluginLanguage language)
		{
			switch (language)
			{
			case XYZ::Utils::PluginLanguage::None:
				XYZ_ASSERT(false, "");
				return "None";
			case XYZ::Utils::PluginLanguage::CPP:
				return "CPP";
			case XYZ::Utils::PluginLanguage::CS:
				return "CS";
			}
			XYZ_ASSERT(false, "");
			return "";
		}

		PluginLanguage StringToPluginLanguage(const std::string& str)
		{
			if (str == "CPP")
				return PluginLanguage::CPP;
			if (str == "CS")
				return PluginLanguage::CS;

			XYZ_ASSERT(false, "");
			return PluginLanguage::None;
		}


		struct Plugin
		{
			std::string		PluginName;
			std::string		BinaryPath;
			PluginLanguage	Language;
		};

		static std::string PluginBinaryPath(const std::filesystem::path& pluginDirectory, const std::string& pluginName)
		{
			std::string result = pluginDirectory.string() + "/bin";
			result += "/" + std::string(XYZ_BINARY_DIR);
			result += "/" + pluginName;

#ifdef XYZ_PLATFORM_WINDOWS
			result += ".dll";
#elif XYZ_PLATFORM_UNIX
			result += ".so";
#endif
			return result;
		}

		static std::filesystem::path FindPluginInDirectory(const std::filesystem::path& dir)
		{
			for (auto it : dir)
			{
				if (it.extension() == ".xyzplugin")
					return it;
			}
		}


		static Plugin LoadPluginInDirectory(const std::filesystem::path& dir)
		{
			std::filesystem::path pluginPath = FindPluginInDirectory(dir);

			std::ifstream stream(pluginPath);
			std::stringstream strStream;
			strStream << stream.rdbuf();
			YAML::Node data = YAML::Load(strStream.str());

			Plugin result;
			result.PluginName = data["PluginName"].as<std::string>();
			result.Language = StringToPluginLanguage(data["Language"].as<std::string>());
			result.BinaryPath = PluginBinaryPath(dir, result.PluginName);
			return result;
		}
	}


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
		for (const auto& path : s_ActiveProject->Configuration.PluginPaths)
		{
			Utils::Plugin plugin = Utils::LoadPluginInDirectory(path);
			if (plugin.Language == Utils::PluginLanguage::CPP)
			{
				PluginManager::ClosePlugin(plugin.BinaryPath);
			}
		}

		s_ActiveProject = ProjectSerializer::Deserialize(path);
		for (const auto& path: s_ActiveProject->Configuration.PluginPaths)
		{
			Utils::Plugin plugin = Utils::LoadPluginInDirectory(path);
			if (plugin.Language == Utils::PluginLanguage::CPP)
			{
				PluginManager::OpenPlugin(plugin.BinaryPath);
			}
			else if (plugin.Language == Utils::PluginLanguage::CS)
			{
				ScriptEngine::LoadRuntimeAssembly(plugin.BinaryPath);
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
		for (const auto& path : s_ActiveProject->Configuration.PluginPaths)
		{
			Utils::Plugin plugin = Utils::LoadPluginInDirectory(path);
			if (plugin.Language == Utils::PluginLanguage::CPP)
			{
				PluginManager::ClosePlugin(plugin.BinaryPath);
			}
		}
		for (const auto& path : s_ActiveProject->Configuration.PluginPaths)
		{
			Utils::Plugin plugin = Utils::LoadPluginInDirectory(path);
			if (plugin.Language == Utils::PluginLanguage::CPP)
			{
				PluginManager::OpenPlugin(plugin.BinaryPath);
			}
			else if (plugin.Language == Utils::PluginLanguage::CS)
			{
				ScriptEngine::LoadRuntimeAssembly(plugin.BinaryPath);
			}
		}
	}

}