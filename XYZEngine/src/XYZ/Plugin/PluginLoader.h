#pragma once


namespace XYZ {
	class XYZ_API PluginLoader
	{
	public:
		static void* LoadPlugin(const std::string& filepath);
		static void  UnloadPlugin(void* handle);

		static void* LoadFunction(void* handle, const std::string& name);

	};
}