#include "stdafx.h"
#include "PluginManager.h"
#include "PluginLoader.h"
#include "Plugin.h"


#define CR_MAIN_FUNC "EntryPoint"
#define CR_HOST

#ifdef XYZ_DEBUG
	#define CR_DEBUG
#endif


#include <cr.h>

namespace XYZ {

	typedef PluginInterface* (*PluginCreatePoint)();
	typedef void(*PluginDestroyPoint)(PluginInterface* plugin);

#define CREATE_PLUGIN_FUNCTION "CreatePlugin"
#define DESTROY_PLUGIN_FUNCTION "DestroyPlugin";

	struct PluginRuntime
	{
		PluginInterface* Plugin;
		void* Handle;
	};


	static std::unordered_map<std::string, PluginRuntime> s_PluginsRuntime;

	static bool LoadPluginRuntime(PluginRuntime& runtime, const std::string& filepath)
	{
		runtime.Handle = PluginLoader::LoadPlugin(filepath);
		if (runtime.Handle == nullptr)
			return false;

		PluginCreatePoint createFunction = (PluginCreatePoint)PluginLoader::LoadFunction(runtime.Handle, CREATE_PLUGIN_FUNCTION);
		if (createFunction == nullptr)
			return false;

		runtime.Plugin = createFunction();
		if (runtime.Plugin == nullptr)
			return false;
	}


	void PluginManager::OpenPlugin(const std::string& filepath)
	{
		PluginRuntime plugin;
		if (LoadPluginRuntime(plugin, filepath))
		{
			s_PluginsRuntime[filepath] = plugin;
			plugin.Plugin->OnCreate();
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
			runtime.Plugin->OnDestroy();
	
			PluginDestroyPoint destroyFunction = (PluginDestroyPoint)PluginLoader::LoadFunction(runtime.Handle, CREATE_PLUGIN_FUNCTION);
			if (destroyFunction == nullptr)
			{
				XYZ_CORE_WARN("Failed to destroy plugin {}", filepath);
			}
			else
			{
				destroyFunction(runtime.Plugin);
			}


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
			it.second.Plugin->OnUpdate(ts);
		}
	}
}