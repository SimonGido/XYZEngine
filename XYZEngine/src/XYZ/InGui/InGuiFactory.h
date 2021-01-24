#pragma once

#include "InGui.h"

namespace XYZ {

	class InGuiFactory
	{
	public:
		static void GenerateWindow(const char* text, InGuiWindow& window, const glm::vec4& color, const InGuiRenderData& renderData);
		static void GenerateQuad(InGuiWindow& window, const glm::vec4& color, const glm::vec2& size, const glm::vec2& position, const InGuiRenderData& renderData, uint32_t subTextureIndex);
		static void GenerateQuad(InGuiWindow& window, const glm::vec4& color, const glm::vec2& size, const glm::vec2& position, Ref<SubTexture> subTexture, uint32_t textureID);
		static glm::vec2 GenerateQuadWithText(const char* text, InGuiWindow& window, const glm::vec4& color, const glm::vec2& size, const glm::vec2& position, const InGuiRenderData& renderData, uint32_t subTextureIndex);
		static glm::vec2 GenerateQuadWithTextLeft(const char* text, InGuiWindow& window, const glm::vec4& color, const glm::vec2& size, const glm::vec2& position, const InGuiRenderData& renderData, uint32_t subTextureIndex);
		static glm::vec2 GenerateTextCentered(const char* text, InGuiWindow& window, const glm::vec2& position, const glm::vec2& size, const InGuiRenderData& renderData, uint32_t maxCount);
		static glm::vec2 GenerateText(const char* text, InGuiWindow& window, const glm::vec4& color, const glm::vec2& position, const glm::vec2& size, const InGuiRenderData& renderData);
	};
}