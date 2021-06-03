#pragma once
#include "InGuiDrawList.h"

#include <stdarg.h>

namespace XYZ {
	namespace Util{

		void GenerateTextMesh(
			const char* source,
			const Ref<Font>& font,
			const glm::vec4& color,
			const glm::vec2& pos,
			std::vector<InGuiQuad>& quads,
			uint32_t textureID,
			uint32_t clipID,
			char ignore = '#'
		);

		void GenerateTextMeshClipped(
			const char* source,
			const Ref<Font>& font,
			const glm::vec4& color,
			const glm::vec2& pos,
			std::vector<InGuiQuad>& quads,
			uint32_t textureID,
			uint32_t clipID,
			const glm::vec2 size,
			char ignore = '#'
		);

		uint32_t CalculateNumCharacters(
			const char* source,
			const Ref<Font>& font,
			const glm::vec2 size,
			char ignore = '#'
		);

		glm::vec2 CalculateTextSize(
			const char* source,
			const Ref<Font>& font,
			char ignore = '#'
		);

		glm::vec2 CalculateTextSize(
			const char* source,
			const Ref<Font>& font,
			uint32_t maxCharacters,
			char ignore = '#'
		);

		int  FormatString(char* buf, size_t bufSize, const char* fmt, ...);	
		void FormatFloat(char* buffer, size_t bufSize, float value, int decimalPrecision);
	}
}