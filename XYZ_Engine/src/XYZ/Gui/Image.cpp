#include "stdafx.h"
#include "Image.h"


namespace XYZ {
	Image::Image(const glm::vec2& position, const glm::vec2& size, const glm::vec4& color, int32_t textureID, Ref<SubTexture2D> subTexture, Ref<Material> material, SortLayerID layer, bool visible)
		:
		m_Quad(position, size, color, textureID),
		RenderComponent(material, layer, visible)
	{
	}
	void Image::SetSubTexture(Ref<SubTexture2D> subTexture)
	{
	}
	void Image::SetQuad(const Quad& quad)
	{
	}
}