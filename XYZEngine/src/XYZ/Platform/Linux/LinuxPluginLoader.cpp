#include "stdafx.h"

#ifdef XYZ_PLATFORM_LINUX
#include "XYZ/Plugin/PluginLoader.h"

namespace XYZ {

    void* PluginLoader::LoadPlugin(const std::string& filepath)
    {
        dlerror();
        auto new_dll = dlopen(filepath.c_str(), RTLD_NOW);
        if (!new_dll) 
        {

            XYZ_CORE_ERROR("Couldn't load plugin: {}", dlerror());
        }
        return new_dll;
    }

    void PluginLoader::UnloadPlugin(void* handle)
    {
        const int r = dlclose(p->handle);
        if (r) 
        {
            XYZ_CORE_ERROR("Error closing plugin: {}", r);
        }
    }

    
	void* PluginLoader::LoadFunction(void* handle, const std::string& name)
	{
        dlerror();
        auto func = dlsym(handle, name.c_str());
        if (!func) 
        {
            XYZ_CORE_ERROR("Could not load function {}", name);
        }
        return func;
	}
}

#endif