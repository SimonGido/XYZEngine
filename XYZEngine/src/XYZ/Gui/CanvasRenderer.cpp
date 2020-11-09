#include "stdafx.h"
#include "CanvasRenderer.h"


namespace XYZ {
	CanvasRenderer::CanvasRenderer(
		Ref<XYZ::Material> material,
		const XYZ::Mesh& mesh,
		uint32_t textureID,
		int32_t sortLayer,
		bool isVisible
	)
		:
		Material(material),
		Mesh(mesh),
		TextureID(textureID),
		SortLayer(sortLayer),
		IsVisible(isVisible)
	{
	}
	CanvasRenderer::CanvasRenderer(const CanvasRenderer& other)
		:
		Material(other.Material),
		Mesh(other.Mesh),
		TextureID(other.TextureID),
		SortLayer(other.SortLayer),
		IsVisible(other.IsVisible)
	{
	}
	CanvasRenderer::CanvasRenderer(CanvasRenderer&& other) noexcept
		:
		Material(other.Material),
		Mesh(other.Mesh),
		TextureID(other.TextureID),
		SortLayer(other.SortLayer),
		IsVisible(other.IsVisible)
	{
	}
	CanvasRenderer& CanvasRenderer::operator=(const CanvasRenderer& other)
	{
		Material = other.Material;
		Mesh = other.Mesh;
		TextureID = other.TextureID;
		SortLayer = other.SortLayer;
		IsVisible = other.IsVisible;

		return *this;
	}
}