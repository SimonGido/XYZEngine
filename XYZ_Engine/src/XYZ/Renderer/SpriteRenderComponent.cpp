#include "stdafx.h"
#include "SpriteRenderComponent.h"

namespace XYZ {
	SpriteRenderComponent::SpriteRenderComponent(
		const glm::vec2& position,
		const glm::vec2& size,
		const glm::vec4& color,
		int32_t textureID,
		Ref<Material> material,
		SortLayerID layer,
		bool visible
	)
		:
		m_Quad(position, size, color,textureID),
		RenderComponent(material,layer, visible)
	{
	}
	void SpriteRenderComponent::SetSubTexture(Ref<SubTexture2D> subTexture)
	{
		m_SubTexture = subTexture;
		auto &texcoords = subTexture->GetTexCoords();

		m_Quad.Vertices[0].TexCoord = { texcoords.x,texcoords.y };
		m_Quad.Vertices[1].TexCoord = { texcoords.z,texcoords.y };
		m_Quad.Vertices[2].TexCoord = { texcoords.z,texcoords.w };
		m_Quad.Vertices[3].TexCoord = { texcoords.x,texcoords.w };
	}
	void SpriteRenderComponent::SetQuad(const Quad& quad)
	{
		m_Quad = quad;
	}
}