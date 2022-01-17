#include "stdafx.h"
#include "SpriteSheet.h"


namespace XYZ {

	UV Calculate(const glm::vec2& coords, const glm::vec2& cellSize, const glm::vec2& textureSize)
	{
		UV uv;
		uv[0] = { (coords.x * cellSize.x) / textureSize.x,
				 ((coords.y + 1) * cellSize.y) / textureSize.y };
		uv[1] = { ((coords.x + 1) * cellSize.x) / textureSize.x,
				   (coords.y * cellSize.y) / textureSize.y };

		return uv;
	}

	SpriteSheet::SpriteSheet(const Ref<Texture2D>& texture, uint32_t columns, uint32_t rows)
		:
		m_Texture(texture),
		m_Columns(columns),
		m_Rows(rows)
	{
		const float width = (float)m_Texture->GetWidth();
		const float height = (float)m_Texture->GetHeight();
		const glm::vec2 cellSize = glm::vec2(width / m_Columns, height / m_Rows); 
		const glm::vec2 textureSize = { width, height };

		m_UVs.reserve(m_Columns * m_Rows);
		for (uint32_t row = 0; row < m_Rows; ++row)
		{
			for (uint32_t column = 0; column < m_Columns; ++column)
				m_UVs.push_back(Calculate(glm::vec2(column, row), cellSize, textureSize));
		}
	}
	const UV& SpriteSheet::GetTexCoords(uint32_t index) const
	{
		return m_UVs[index];
	}
	const UV& SpriteSheet::GetTexCoords(uint32_t column, uint32_t row) const
	{
		return m_UVs[(m_Columns * row) + column];
	}

}