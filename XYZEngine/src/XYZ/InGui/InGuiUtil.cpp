#include "InGuiUtil.h"


namespace XYZ {

	namespace Util {
		void GenerateTextMesh(const char* source, const Ref<Font>& font, const glm::vec4& color, const glm::vec2& pos, std::vector<InGuiQuad>& quads, uint32_t textureID, uint32_t clipID)
		{
			if (!source)
				return;

			float xCursor = 0.0f;
			float yCursor = 0.0f;

			uint32_t counter = 0;
			while (source[counter] != '\0')
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
		void GenerateTextMeshClipped(const char* source, const Ref<Font>& font, const glm::vec4& color, const glm::vec2& pos, std::vector<InGuiQuad>& quads, uint32_t textureID, uint32_t clipID, const glm::vec2 size)
		{
			if (!source)
				return;

			float xCursor = 0.0f;
			float yCursor = 0.0f;

			uint32_t counter = 0;
			while (source[counter] != '\0' && yCursor < size.y)
			{
				auto& character = font->GetCharacter(source[counter]);
				if (xCursor + character.XAdvance >= size.x)
					break;

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
		uint32_t CalculateNumCharacters(const char* source, const Ref<Font>& font, const glm::vec2 size)
		{
			if (!source)
				return 0;

			float width = 0.0f;
			float xCursor = 0.0f;
			float yCursor = 0.0f;

			uint32_t counter = 0;
			while (source[counter] != '\0' && yCursor < size.y)
			{
				auto& character = font->GetCharacter(source[counter]);
				if (xCursor + character.XAdvance >= size.x)
					break;

				if (source[counter] == '\n')
				{
					width = std::max(width, xCursor);
					yCursor += font->GetLineHeight();
					xCursor = 0.0f;
					counter++;
					continue;
				}
				xCursor += character.XAdvance;
				counter++;
			}
			return counter;
		}
		glm::vec2 CalculateTextSize(const char* source, const Ref<Font>& font)
		{
			if (!source)
				return { 0.0f, 0.0f };

			float width = 0.0f;
			float xCursor = 0.0f;
			float yCursor = 0.0f;

			uint32_t counter = 0;
			while (source[counter] != '\0')
			{
				auto& character = font->GetCharacter(source[counter]);
				if (source[counter] == '\n')
				{
					width = std::max(width, xCursor);
					yCursor += font->GetLineHeight();
					xCursor = 0.0f;
					counter++;
					continue;
				}
				xCursor += character.XAdvance;
				counter++;
			}
			if (width < xCursor)
				width = xCursor;
			return { width, yCursor + font->GetLineHeight() };
		}
		glm::vec2 CalculateTextSize(const char* source, const Ref<Font>& font, uint32_t maxCharacters)
		{
			if (!source)
				return { 0.0f, 0.0f };

			float width = 0.0f;
			float xCursor = 0.0f;
			float yCursor = 0.0f;

			uint32_t counter = 0;
			while (source[counter] != '\0' && counter < maxCharacters)
			{
				auto& character = font->GetCharacter(source[counter]);
				if (source[counter] == '\n')
				{
					width = std::max(width, xCursor);
					yCursor += font->GetLineHeight();
					xCursor = 0.0f;
					counter++;
					continue;
				}
				xCursor += character.XAdvance;
				counter++;
			}
			if (width < xCursor)
				width = xCursor;
			return { width, yCursor + font->GetLineHeight() };
		}
	}
}