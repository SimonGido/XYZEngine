#pragma once
#include "XYZ/Core/Timestep.h"

#include "Plugin.h"

namespace XYZ {
	class XYZ_API PluginManager
	{
	public:
		static void OpenPlugin(const std::string& filepath);
		static void ClosePlugin(const std::string& filepath);
		static void CloseAll();

		static void Update(Timestep ts);
	};
}