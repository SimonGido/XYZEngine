#include "stdafx.h"
#include "SubTexture.h"



namespace XYZ {
	SubTexture::SubTexture(const Ref<Texture>& texture, const glm::vec2& coords, const glm::vec2& size)
		: m_Texture(texture)
	{
		glm::vec2 min = { (coords.x * size.x) / texture->GetWidth(), (coords.y * size.y) / texture->GetHeight() };
		glm::vec2 max = { (coords.x + 1) * size.x / texture->GetWidth(), ((coords.y + 1) * size.x) / texture->GetHeight() };
		
		m_TexCoords = { min, max };
	}

	SubTexture::SubTexture(const Ref<Texture>& texture, const glm::vec4& texCoords)
		: m_Texture(texture), m_TexCoords(texCoords)
	{
	
	}

	void SubTexture::SetCoordsWithSize(const glm::vec2& coords, const glm::vec2& size)
	{
		glm::vec2 min = { (coords.x * size.x) / m_Texture->GetWidth(), (coords.y * size.y) / m_Texture->GetHeight() };
		glm::vec2 max = { (coords.x + 1) * size.x / m_Texture->GetWidth(), ((coords.y + 1) * size.x) / m_Texture->GetHeight() };

		m_TexCoords = { min, max };
	}
}