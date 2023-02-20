#pragma once
#include "Core.h"

namespace XYZ {
	class XYZ_API Platform
	{
	public:
		static bool ExecuteCommand(std::string app, const std::string& args, std::string& output);

	};
}