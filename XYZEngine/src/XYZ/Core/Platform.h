#pragma once
#include "Core.h"

namespace XYZ {
	class XYZ_API Platform
	{
	public:
		std::string RunShellCommand(const char* command);
	};
}