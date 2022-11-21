#include "stdafx.h"
#include "PluginManager.h"

#define CR_MAIN_FUNC "EntryPoint"
#define CR_HOST
#ifdef XYZ_DEBUG
	#define CR_DEBUG
#endif


#include <cr.h>

namespace XYZ {
	struct Plugin
	{
		std::string Filepath;
		cr_plugin   Context;
	};

	static std::vector<Plugin> s_Plugins;


	void PluginManager::OpenPlugin(const std::string& path)
	{
		Plugin plugin;
		plugin.Filepath = path;
		if (cr_plugin_open(plugin.Context, plugin.Filepath.c_str()))
		{
			s_Plugins.push_back(plugin);
		}
		else
		{
			XYZ_CORE_WARN("Failed to open plugin {}", plugin.Filepath);
		}
	}

	void PluginManager::ClosePlugin(const std::string& path)
	{
		for (auto it = s_Plugins.begin(); it != s_Plugins.end(); ++it)
		{
			if (it->Filepath == path)
			{
				cr_plugin_close(it->Context);
				s_Plugins.erase(it);
				return;
			}
		}
	}

	void PluginManager::Update(Timestep ts)
	{
		for (auto& plugin : s_Plugins)
		{
			int result = cr_plugin_update(plugin.Context);
		}
	}

}