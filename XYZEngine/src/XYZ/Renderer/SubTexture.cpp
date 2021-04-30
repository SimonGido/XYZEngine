#include "stdafx.h"
#include "SubTexture.h"



namespace XYZ {
	SubTexture::SubTexture(const Ref<Texture>& texture, const glm::vec2& coords, const glm::vec2& size)
		: 
		m_Texture(texture),
		m_Coords(coords),
		m_Size(size)
	{		
		calculatePixelCorrectedTexCoords();
	}
	SubTexture::SubTexture(const Ref<Texture>& texture, const glm::vec2& coords)
		:
		m_Texture(texture),
		m_Coords(coords)
	{
		m_Size.x = (float)texture->GetWidth();
		m_Size.y = (float)texture->GetHeight();
		calculatePixelCorrectedTexCoords();
	}
	void SubTexture::SetCoords(const glm::vec2& coords, const glm::vec2& size)
	{
		m_Coords = coords;
		m_Size = size;
	}
	const glm::vec4& SubTexture::GetTexCoords() const
	{
		return m_TexCoords;
	}

	void SubTexture::calculatePixelCorrectedTexCoords()
	{
		m_TexCoords = {  
			((m_Coords.x      * m_Size.x)) / m_Texture->GetWidth(),  ((m_Coords.y      * m_Size.y)) / m_Texture->GetHeight(),
			(((m_Coords.x + 1) * m_Size.x)) / m_Texture->GetWidth(), (((m_Coords.y + 1) * m_Size.y)) / m_Texture->GetHeight() 
		};
	}

}