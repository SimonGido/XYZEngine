#pragma once

#include "InGui.h"

namespace XYZ {

	class InGuiFactory
	{
	public:
		static void GenerateWindow(const char* text, InGuiWindow& window, const glm::vec4& color, const InGuiRenderData& renderData);
		static glm::vec2 GenerateQuadWithText(const char* text, InGuiWindow& window, const glm::vec4& color, const glm::vec2& size, const glm::vec2& position, const InGuiRenderData& renderData);
	};
}