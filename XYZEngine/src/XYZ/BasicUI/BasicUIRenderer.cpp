#include "stdafx.h"
#include "BasicUIRenderer.h"

#include "BasicUITypes.h"
#include "BasicUI.h"

namespace XYZ {
	namespace Helper {
		static void GenerateQuad(bUIMesh& mesh, const glm::vec4& color, const glm::vec2& size, const glm::vec2& position, const Ref<SubTexture>& subTexture, uint32_t textureID)
		{
			const glm::vec4& oldTex = subTexture->GetTexCoords();
			glm::vec4 texCoords = {
				oldTex.x, oldTex.w, oldTex.z, oldTex.y
			};
			mesh.Quads.push_back(
				{
					color,
					texCoords,
					{position, 0.0f},
					size,
					textureID,
				});
		}

		static glm::vec2 FindTextSize(const char* source, const Ref<Font>& font)
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
					width = xCursor;
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

		static void GenerateTextMesh(
			const char* source,
			const Ref<Font>& font,
			const glm::vec4& color,
			const glm::vec2& pos,
			bUIMesh& mesh,
			uint32_t textureID
		)
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

				mesh.Quads.push_back({ color, charTexCoord, glm::vec3{charPosition, 0.0f}, charSize, textureID });
				xCursor += character.XAdvance;
				counter++;
			}
		}
	}

	template <>
	void bUIRenderer::Submit<bUIButton>(const bUIButton& element, const Ref<SubTexture>& subTexture)
	{
		glm::vec2 absolutePosition = element.GetAbsolutePosition();
		Ref<Font> font = bUI::GetConfig().m_Font;
		Helper::GenerateQuad(m_Mesh, element.ActiveColor, element.Size, absolutePosition, subTexture, 0);
		glm::vec2 size = Helper::FindTextSize(element.Label.c_str(), font);
		glm::vec2 textPosition = absolutePosition;
		textPosition.x += (element.Size.x / 2.0f) - (size.x / 2.0f);
		textPosition.y += (element.Size.y / 2.0f) + (size.y / 2.0f);
		Helper::GenerateTextMesh(element.Label.c_str(), font, glm::vec4(1.0f), textPosition, m_Mesh, 1);
	}
	void bUIRenderer::Begin()
	{
		m_Mesh.Quads.clear();
		m_Mesh.Lines.clear();
	}
}