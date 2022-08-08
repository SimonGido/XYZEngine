#pragma once
#include "SubTexture.h"



namespace XYZ {
	using UV = std::array<glm::vec2, 2>;
	
	UV Calculate(const glm::vec2& coords, const glm::vec2& size, const glm::vec2& textureSize);
	
	
	class SpriteSheet : public RefCount
	{
	public:
		SpriteSheet(const Ref<Texture2D>& texture, uint32_t columns, uint32_t rows);

		const UV& GetTexCoords(uint32_t index) const;
		const UV& GetTexCoords(uint32_t column, uint32_t row) const;

		
		const Ref<Texture2D>& GetTexture() const { return m_Texture; }
		uint32_t			  GetColumns() const { return m_Columns; }
		uint32_t			  GetRows()	   const { return m_Rows; }
	private:
		Ref<Texture2D> m_Texture;
		const uint32_t m_Columns;
		const uint32_t m_Rows;

		std::vector<UV> m_UVs;
	};
}