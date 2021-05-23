#pragma once
#include "InGuiContext.h"

namespace XYZ {

	class InGuiSerializer
	{
	public:
		static void Serialize(const InGuiContext& context, const std::string& filepath);
		static void Deserialize(InGuiContext& context, const std::string& filepath);
	};

}