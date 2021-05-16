#pragma once
#include "InGuiDrawList.h"

namespace XYZ {
	class Util
	{
	public:
		static void InGuiGenerateTextMesh(
			const char* source,
			const Ref<Font>& font,
			const glm::vec4& color,
			const glm::vec2& pos,
			std::vector<InGuiQuad>& quads,
			uint32_t textureID,
			uint32_t clipID,
			uint32_t maxCharacters = UINT32_MAX
		);
	};
}