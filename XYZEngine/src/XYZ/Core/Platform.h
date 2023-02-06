#pragma once
#include "Core.h"

namespace XYZ {
	class XYZ_API Platform
	{
	public:
		static std::string RunShellCommand(const char* command);
	};
}