#include "stdafx.h"
#include "PluginManager.h"
#include "PluginLoader.h"

#include "XYZ/Asset/Asset.h"

#define CR_MAIN_FUNC "EntryPoint"
#define CR_HOST

#ifdef XYZ_DEBUG
	#define CR_DEBUG
#endif


#include <cr.h>

namespace XYZ {

	typedef void (*PluginLoadPoint)();
	typedef void (*PluginClosePoint)();
	typedef void (*PluginUpdatePoint)(Timestep ts);

#define BEGIN_FUNCTION "OnLoad"
#define END_FUNCTION "OnClose"
#define UPDATE_FUNCTION "OnUpdate"

	struct PluginRuntime
	{
		PluginLoadPoint  OnLoad;
		PluginClosePoint OnClose;
		PluginUpdatePoint OnUpdate;
		void* Handle;
	};


	static std::unordered_map<std::string, PluginRuntime> s_PluginsRuntime;

	static bool LoadPluginRuntime(PluginRuntime& plugin, const std::string& filepath)
	{
		plugin.Handle = PluginLoader::LoadPlugin(filepath);
		if (plugin.Handle == nullptr)
			return false;

		plugin.OnLoad = (PluginLoadPoint)PluginLoader::LoadFunction(plugin.Handle, BEGIN_FUNCTION);
		if (plugin.OnLoad == nullptr)
			return false;

		plugin.OnClose = (PluginClosePoint)PluginLoader::LoadFunction(plugin.Handle, END_FUNCTION);
		if (plugin.OnClose == nullptr)
			return false;

		plugin.OnUpdate = (PluginUpdatePoint)PluginLoader::LoadFunction(plugin.Handle, UPDATE_FUNCTION);
		if (plugin.OnUpdate == nullptr)
			return false;
	}


	void PluginManager::OpenPlugin(const std::string& filepath)
	{
		PluginRuntime plugin;
		if (LoadPluginRuntime(plugin, filepath))
		{
			s_PluginsRuntime[filepath] = plugin;
			plugin.OnLoad();
		}
		else
		{
			XYZ_CORE_WARN("Failed to open plugin {}", filepath);
		}

	}

	void PluginManager::ClosePlugin(const std::string& filepath)
	{
		auto it = s_PluginsRuntime.find(filepath);
		if (it != s_PluginsRuntime.end())
		{
			PluginRuntime& runtime = it->second;
			runtime.OnClose();
			PluginLoader::UnloadPlugin(runtime.Handle);
			s_PluginsRuntime.erase(it);
		}
	}

	void PluginManager::CloseAll()
	{
		while (!s_PluginsRuntime.empty())
		{
			auto it = s_PluginsRuntime.begin();
			ClosePlugin(it->first);
		}
	}

	void PluginManager::Update(Timestep ts)
	{
		for (auto &it : s_PluginsRuntime)
		{
			it.second.OnUpdate(ts);
		}
	}

}