#include "InGuiUtil.h"


namespace XYZ {

	void Util::InGuiGenerateTextMesh(const char* source, const Ref<Font>& font, const glm::vec4& color, const glm::vec2& pos, std::vector<InGuiQuad>& quads, uint32_t textureID, uint32_t clipID, uint32_t maxCharacters)
	{
		if (!source)
			return;

		float xCursor = 0.0f;
		float yCursor = 0.0f;

		uint32_t counter = 0;
		while (source[counter] != '\0' && counter < maxCharacters)
		{
			auto& character = font->GetCharacter(source[counter]);
			if (source[counter] == '\n')
			{
				yCursor += font->GetLineHeight();
				xCursor = 0.0f;
				counter++;
				continue;
			}
			glm::vec2 charSize = {
				character.X1Coord - character.X0Coord,
				character.Y1Coord - character.Y0Coord
			};

			glm::vec2 charOffset = { character.XOffset, character.YOffset };
			glm::vec2 charPosition = { pos.x + xCursor + charOffset.x, pos.y + yCursor - charOffset.y };
			glm::vec4 charTexCoord = {
				(float)(character.X0Coord) / (float)(font->GetWidth()), (float)(character.Y0Coord) / (float)(font->GetHeight()),
				(float)(character.X1Coord) / (float)(font->GetWidth()), (float)(character.Y1Coord) / (float)(font->GetHeight())
			};

			quads.push_back({ color, charTexCoord, glm::vec3{charPosition, 0.0f}, charSize, textureID, clipID });
			xCursor += character.XAdvance;
			counter++;
		}
	}
}