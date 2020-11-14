#pragma once
#include "stdafx.h"
#include "Components.h"



namespace XYZ {
	SpriteRenderer::SpriteRenderer(XYZ::Ref<XYZ::Material> material, Ref<XYZ::SubTexture2D> subTexture, const glm::vec4& color, int32_t sortLayer, bool isVisible)
		:
		Material(material),
		SubTexture(subTexture),
		Color(color),
		SortLayer(sortLayer),
		IsVisible(isVisible)
	{}

	SpriteRenderer::SpriteRenderer(const XYZ::SpriteRenderer& other)
		:
		Material(other.Material),
		SubTexture(other.SubTexture),
		Color(other.Color),
		SortLayer(other.SortLayer),
		IsVisible(other.IsVisible)
	{
	}

	SpriteRenderer::SpriteRenderer(XYZ::SpriteRenderer&& other) noexcept
		:
		Material(other.Material),
		SubTexture(other.SubTexture),
		Color(other.Color),
		SortLayer(other.SortLayer),
		IsVisible(other.IsVisible)
	{
	}

	SpriteRenderer& XYZ::SpriteRenderer::operator=(const XYZ::SpriteRenderer& other)
	{
		Material = other.Material;
		SubTexture = other.SubTexture;
		Color = other.Color;
		SortLayer = other.SortLayer;
		IsVisible = other.IsVisible;

		return *this;
	}

}