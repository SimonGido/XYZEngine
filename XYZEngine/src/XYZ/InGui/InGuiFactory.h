#pragma once

#include "InGui.h"

namespace XYZ {

	class InGuiFactory
	{
	public:
		static void GenerateWindow(const char* text, InGuiWindow& window, const glm::vec4& color, const InGuiRenderData& renderData);

	};
}