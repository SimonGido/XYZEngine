#include "stdafx.h"
#include "SubTexture.h"



namespace XYZ {

	SubTexture::SubTexture(const Ref<Texture2D>& texture, const glm::vec4& texCoords)
		:
		m_Texture(texture),
		m_TexCoords(texCoords)
	{
	}
	SubTexture::SubTexture(const Ref<Texture2D>& texture)
		:
		m_Texture(texture),
		m_TexCoords(0.0f, 0.0f, 1.0f, 1.0f)
	{

	}
	void SubTexture::SetTexCoords(const glm::vec4& texCoords)
	{
		m_TexCoords = texCoords;
	}
	void SubTexture::Upside()
	{
		m_TexCoords = { m_TexCoords.x, m_TexCoords.w, m_TexCoords.z, m_TexCoords.y };
	}
	const glm::vec4& SubTexture::GetTexCoords() const
	{
		return m_TexCoords;
	}

	glm::vec4 SubTexture::GetTexCoordsUpside() const
	{
		return { m_TexCoords.x, m_TexCoords.w, m_TexCoords.z, m_TexCoords.y };
	}
}