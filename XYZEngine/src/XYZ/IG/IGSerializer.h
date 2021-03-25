#pragma once
#include "IGContext.h"


namespace XYZ {

	class IGSerializer
	{
	public:
		static void Serialize(const char* filepath, const IGContext& context);
		static void Deserialize(const char* filepath, IGContext& context);
	};
}