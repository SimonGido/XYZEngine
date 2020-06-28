#include "stdafx.h"
#include "Text.h"


namespace XYZ {
	Text::Text(const std::string& text, const glm::vec2& position, const glm::vec4& color, int32_t textureID, Ref<Font> font, Ref<Material> material, SortLayerID layer, bool visible)
		:
		m_Text(text),
		m_Position(position),
		m_Color(color),
		m_TextureID(textureID),
		m_Font(font),
		RenderComponent(material, layer, visible)
	{
		rebuild();
	}
	void Text::Pop()
	{
	}
	void Text::Push(char c)
	{
	}
	void Text::Push(const std::string& text)
	{
	}
	void Text::Clear()
	{
		m_Text.clear();
		m_Quads.clear();
		m_CursorX = 0;
		m_CursorY = 0;
	}
	void Text::rebuild()
	{
		auto& fontData = m_Font->GetData();
		m_Quads.reserve(m_Text.size());
		for (auto c : m_Text)
		{
			auto &character = m_Font->GetCharacter(c);
			glm::vec2 position = {
				m_Position.x + m_CursorX + character.XOffset,
				m_Position.y + m_CursorY + (-character.YOffset - (int32_t)character.Height)
			};
			

			glm::vec2 size = { character.Width, character.Height };
			Quad quad(position, size, m_Color, m_TextureID, true);
			
				
			glm::vec2 coords = { character.XCoord, fontData.ScaleH - character.YCoord - character.Height };
			glm::vec2 scale =  { fontData.ScaleW, fontData.ScaleH };

			quad.Vertices[0].TexCoord = (coords) / scale;
			quad.Vertices[1].TexCoord = (coords + glm::vec2(character.Width, 0)) / scale;
			quad.Vertices[2].TexCoord = (coords + glm::vec2(character.Width, character.Height)) / scale;
			quad.Vertices[3].TexCoord = (coords + glm::vec2(0, character.Height)) / scale;

			m_CursorX += character.XAdvance;
			m_Quads.push_back(quad);
		}
	}
}