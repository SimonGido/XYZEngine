#include "stdafx.h"
#include "Text.h"


namespace XYZ {
	Text::Text(const std::string& text, const glm::vec2& position, const glm::vec2& size, const glm::vec4& color, int32_t textureID, Ref<Font> font, Ref<Material> material, SortLayerID layer, bool visible)
		:
		m_Text(text),
		m_Position(position),
		m_Size(size),
		m_Color(color),
		m_Font(font),
		RenderComponent(material, layer, visible)
	{
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
	void Text::rebuild()
	{
		for (auto c : m_Text)
		{
			auto &character = m_Font->GetCharacter(c);
			Vertex vertex = {
				m_Color,
				{m_Position.x + m_Cursor,m_Position.y,0,0},
			};
		}
	}
}