#include "stdafx.h"
#include "SpriteRenderer.h"

XYZ::SpriteRenderer::SpriteRenderer(XYZ::Ref<XYZ::Material> material, Ref<XYZ::SubTexture2D> subTexture, const glm::vec4& color, uint32_t textureID, int32_t sortLayer, bool isVisible)
	:
	Material(material),
	SubTexture(subTexture),
	Color(color),
	TextureID(textureID),
	SortLayer(sortLayer),
	IsVisible(isVisible)
{}

XYZ::SpriteRenderer::SpriteRenderer(const XYZ::SpriteRenderer & other)
	:
	Material(other.Material),
	SubTexture(other.SubTexture),
	Color(other.Color),
	TextureID(other.TextureID),
	SortLayer(other.SortLayer),
	IsVisible(other.IsVisible)
{
}

XYZ::SpriteRenderer::SpriteRenderer(XYZ::SpriteRenderer&& other) noexcept
	:
	Material(other.Material),
	SubTexture(other.SubTexture),
	Color(other.Color),
	TextureID(other.TextureID),
	SortLayer(other.SortLayer),
	IsVisible(other.IsVisible)
{
}

XYZ::SpriteRenderer& XYZ::SpriteRenderer::operator=(const XYZ::SpriteRenderer& other)
{
	Material = other.Material;
	SubTexture = other.SubTexture;
	Color = other.Color;
	TextureID = other.TextureID;
	SortLayer = other.SortLayer;
	IsVisible = other.IsVisible;

	return *this;
}
