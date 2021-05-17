#pragma once
#include "InGuiDrawList.h"

namespace XYZ {
	namespace Util{

		void GenerateTextMesh(
			const char* source,
			const Ref<Font>& font,
			const glm::vec4& color,
			const glm::vec2& pos,
			std::vector<InGuiQuad>& quads,
			uint32_t textureID,
			uint32_t clipID
		);

		void GenerateTextMeshClipped(
			const char* source,
			const Ref<Font>& font,
			const glm::vec4& color,
			const glm::vec2& pos,
			std::vector<InGuiQuad>& quads,
			uint32_t textureID,
			uint32_t clipID,
			const glm::vec2 size
		);

		uint32_t CalculateNumCharacters(
			const char* source,
			const Ref<Font>& font,
			const glm::vec2 size
		);

		glm::vec2 CalculateTextSize(
			const char* source,
			const Ref<Font>& font
		);

		glm::vec2 CalculateTextSize(
			const char* source,
			const Ref<Font>& font,
			uint32_t maxCharacters
		);
	}
}