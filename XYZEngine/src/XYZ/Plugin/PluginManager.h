#pragma once
#include "XYZ/Core/Timestep.h"

namespace XYZ {
	class XYZ_API PluginManager
	{
	public:
		static void OpenPlugin(const std::string& path);
		static void ClosePlugin(const std::string& path);

		static void Update(Timestep ts);
	};
}