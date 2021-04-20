#include "stdafx.h"
#include "BasicUIRenderer.h"

#include "BasicUITypes.h"
#include "BasicUI.h"
#include "BasicUIHelper.h"

namespace XYZ {
	namespace Helper {
		static void GenerateQuad(bUIMesh& mesh, const glm::vec4& color, const glm::vec2& size, const glm::vec2& position, const Ref<SubTexture>& subTexture, uint32_t textureID, uint32_t scissorID)
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
					scissorID
				});
		}
		static void GenerateTextMesh(
			const char* source,
			const Ref<Font>& font,
			const glm::vec4& color,
			const glm::vec2& pos,
			bUIMesh& mesh,
			uint32_t textureID,
			uint32_t scissorID
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

				mesh.Quads.push_back({ color, charTexCoord, glm::vec3{charPosition, 0.0f}, charSize, textureID, scissorID });
				xCursor += character.XAdvance;
				counter++;
			}
		}
	}

	template <>
	void bUIRenderer::Submit<bUIButton>(const bUIButton& element, uint32_t& scissorID, const Ref<SubTexture>& subTexture)
	{
		glm::vec2 absolutePosition = element.GetAbsolutePosition();
		Ref<Font> font = bUI::GetConfig().m_Font;
		Helper::GenerateQuad(m_Mesh, element.ActiveColor, element.Size, absolutePosition, subTexture, 0, scissorID);
		glm::vec2 size = bUIHelper::FindTextSize(element.Label.c_str(), font);
		glm::vec2 textPosition = absolutePosition;
		textPosition.x += element.Size.x + 2.0f;
		textPosition.y += (element.Size.y / 2.0f) + (size.y / 2.0f);
		Helper::GenerateTextMesh(element.Label.c_str(), font, glm::vec4(1.0f), textPosition, m_Mesh, 1, scissorID);
	}

	template <>
	void bUIRenderer::Submit<bUICheckbox>(const bUICheckbox& element, uint32_t& scissorID, const Ref<SubTexture>& subTexture)
	{
		glm::vec2 absolutePosition = element.GetAbsolutePosition();
		Ref<Font> font = bUI::GetConfig().m_Font;
		Helper::GenerateQuad(m_Mesh, element.ActiveColor, element.Size, absolutePosition, subTexture, 0, scissorID);
		glm::vec2 size = bUIHelper::FindTextSize(element.Label.c_str(), font);
		glm::vec2 textPosition = absolutePosition;
		textPosition.x += element.Size.x + 2.0f;
		textPosition.y += (element.Size.y / 2.0f) + (size.y / 2.0f);
		Helper::GenerateTextMesh(element.Label.c_str(), font, glm::vec4(1.0f), textPosition, m_Mesh, 1, scissorID);
	}

	template <>
	void bUIRenderer::Submit<bUISlider>(const bUISlider& element, uint32_t& scissorID, const Ref<SubTexture>& sliderSubTexture, const Ref<SubTexture>& handleSubTexture, const glm::vec2& handlePosition, const float& value)
	{
		glm::vec2 absolutePosition = element.GetAbsolutePosition();
		Ref<Font> font = bUI::GetConfig().m_Font;
		Helper::GenerateQuad(m_Mesh, element.ActiveColor, element.Size, absolutePosition, sliderSubTexture, 0, scissorID);
		Helper::GenerateQuad(m_Mesh, element.ActiveColor, { element.Size.y, element.Size.y } , handlePosition, handleSubTexture, 0, scissorID);
		{
			char buffer[10];
			sprintf(buffer, "%f", value);
			glm::vec2 size = bUIHelper::FindTextSize(buffer, font);
			glm::vec2 textPosition = absolutePosition;
			textPosition.x += (element.Size.x / 2.0f) - (size.x / 2.0f);
			textPosition.y += (element.Size.y / 2.0f) + (size.y / 2.0f);
			Helper::GenerateTextMesh(buffer, font, glm::vec4(1.0f), textPosition, m_Mesh, 1, scissorID);
		}
		{
			glm::vec2 size = bUIHelper::FindTextSize(element.Label.c_str(), font);
			glm::vec2 textPosition = absolutePosition;
			textPosition.x += element.Size.x + 2.0f;
			textPosition.y += (element.Size.y / 2.0f) + (size.y / 2.0f);
			Helper::GenerateTextMesh(element.Label.c_str(), font, glm::vec4(1.0f), textPosition, m_Mesh, 1, scissorID);
		}
	}

	template <>
	void bUIRenderer::Submit<bUIGroup>(const bUIGroup& element, uint32_t& scissorID, const Ref<SubTexture>& subTexture, const Ref<SubTexture>& minimizeSubTexture)
	{
		glm::vec2 absolutePosition = element.GetAbsolutePosition();
		Ref<Font> font = bUI::GetConfig().m_Font;
		if (element.ChildrenVisible)
			Helper::GenerateQuad(m_Mesh, element.Color, element.Size, absolutePosition, subTexture, 0, scissorID);
		
		Helper::GenerateQuad(m_Mesh, element.ActiveColor, { element.Size.x, element.ButtonSize.y }, absolutePosition, subTexture, 0,scissorID);
		Helper::GenerateQuad(m_Mesh, element.Color, element.ButtonSize, absolutePosition, minimizeSubTexture, 0, scissorID);
		glm::vec2 size = bUIHelper::FindTextSize(element.Label.c_str(), font);
		glm::vec2 textPosition = absolutePosition;
		textPosition.x += element.ButtonSize.x + 2.0f;
		textPosition.y += (element.ButtonSize.y / 2.0f) + (size.y / 2.0f);
		Helper::GenerateTextMesh(element.Label.c_str(), font, glm::vec4(1.0f), textPosition, m_Mesh, 1, scissorID);
	}

	template <>
	void bUIRenderer::Submit<bUIScrollbox>(const bUIScrollbox& element, uint32_t& scissorID, const Ref<SubTexture>& subTexture)
	{
		glm::vec2 absolutePosition = element.GetAbsoluteScrollPosition();
		Ref<Font> font = bUI::GetConfig().m_Font;
		Helper::GenerateQuad(m_Mesh, element.ActiveColor, element.Size, absolutePosition, subTexture, 0, scissorID);
		glm::vec2 size = bUIHelper::FindTextSize(element.Label.c_str(), font);
		glm::vec2 textPosition = absolutePosition;
		textPosition.x += element.Size.x + 2.0f;
		textPosition.y += (element.Size.y / 2.0f) + (size.y / 2.0f);
		Helper::GenerateTextMesh(element.Label.c_str(), font, glm::vec4(1.0f), textPosition, m_Mesh, 1, scissorID);

		absolutePosition.y = bUI::GetContext().ViewportSize.y - absolutePosition.y - element.Size.y;
		m_Mesh.Scissors.push_back({ absolutePosition.x, absolutePosition.y, element.Size.x, element.Size.y });
	}

	void bUIRenderer::Begin()
	{
		m_Mesh.Quads.clear();
		m_Mesh.Lines.clear();
		m_Mesh.Scissors.clear();
		m_Mesh.Scissors.push_back({ 0.0f, 0.0f, bUI::GetContext().ViewportSize.x, bUI::GetContext().ViewportSize.y });
	}
	void bUIRenderer::UpdateScissorBuffer(Ref<ShaderStorageBuffer> scissorBuffer)
	{
		scissorBuffer->Update(m_Mesh.Scissors.data(), m_Mesh.Scissors.size() * sizeof(bUIScissor));
	}
}