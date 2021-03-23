#pragma once
#include "IGContext.h"


namespace XYZ {

	class IGSerializer
	{
	public:
		static void Serialize(const std::string& filepath, const IGContext& context);
		static void Deserialize(const std::string& filepath, IGContext& context);
	};
}