#include "stdafx.h"

#ifdef XYZ_PLATFORM_WINDOWS
#include "XYZ/Plugin/PluginLoader.h"


namespace XYZ {

    void* PluginLoader::LoadPlugin(const std::string& filepath)
    {
        std::wstring wfilepath(filepath.begin(), filepath.end());
        auto new_dll = LoadLibrary(wfilepath.c_str());
        if (!new_dll) 
        {
            XYZ_ERROR("Couldn't load plugin: {}", GetLastError());
        }
        return new_dll;
    }

    void PluginLoader::UnloadPlugin(void* handle)
    {
        FreeLibrary((HMODULE)handle);
    }

	void *PluginLoader::LoadFunction(void* handle, const std::string& name)
	{
        auto func = GetProcAddress((HMODULE)handle, name.c_str());
        if (!func) 
        {
            XYZ_CORE_ERROR("Could not load function {}", name);
        }
        return func;
	}
}

#endif