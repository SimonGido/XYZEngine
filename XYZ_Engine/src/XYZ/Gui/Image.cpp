#include "stdafx.h"
#include "Image.h"


namespace XYZ {
	Image::Image(int32_t textureID, const glm::vec4& color, Ref<SubTexture2D> subTexture, Ref<Material> material, SortLayerID layer, bool visible)
		:
		m_Quad(textureID,color),
		RenderComponent(material, layer, visible)
	{
		SetSubTexture(subTexture);
	}
	void Image::SetSubTexture(Ref<SubTexture2D> subTexture)
	{
		m_SubTexture = subTexture;
		auto& texcoords = subTexture->GetTexCoords();

		m_Quad.Vertices[0].TexCoord = { texcoords.x,texcoords.y };
		m_Quad.Vertices[1].TexCoord = { texcoords.z,texcoords.y };
		m_Quad.Vertices[2].TexCoord = { texcoords.z,texcoords.w };
		m_Quad.Vertices[3].TexCoord = { texcoords.x,texcoords.w };
	}
	void Image::SetQuad(const Quad& quad)
	{
		m_Quad = quad;
	}
	void Image::SetColor(const glm::vec4& color)
	{
		for (size_t i = 0; i < 4; ++i)
			m_Quad.Vertices[i].Color = color;
	}
}