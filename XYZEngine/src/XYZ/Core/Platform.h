#pragma once
#include "Core.h"

namespace XYZ {
	class XYZ_API Platform
	{
	public:
		static void RunShellCommand(std::string app, const std::string& args);
	};
}