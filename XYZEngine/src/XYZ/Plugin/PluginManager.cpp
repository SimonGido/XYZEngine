#include "stdafx.h"
#include "PluginManager.h"

#define CR_MAIN_FUNC "EntryPoint"

#include <cr.h>

namespace XYZ {
	struct Plugin
	{
		std::string Filepath;
		cr_plugin   Context;
	};

	static std::vector<Plugin> m_Plugins;


	void PluginManager::OpenPlugin(const std::string& path)
	{
		Plugin plugin;
		plugin.Filepath = path;
		if (cr_plugin_open(plugin.Context, plugin.Filepath.c_str()))
		{
			m_Plugins.push_back(plugin);
		}
		else
		{
			XYZ_CORE_WARN("Failed to open plugin {}", plugin.Filepath);
		}
	}

	void PluginManager::ClosePlugin(const std::string& path)
	{
		for (auto it = m_Plugins.begin(); it != m_Plugins.end(); ++it)
		{
			if (it->Filepath == path)
			{
				cr_plugin_close(it->Context);
				m_Plugins.erase(it);
				return;
			}
		}
	}

	void PluginManager::Update(Timestep ts)
	{
		for (auto& plugin : m_Plugins)
		{
			int result = cr_plugin_update(plugin.Context, ts);
		}
	}

}