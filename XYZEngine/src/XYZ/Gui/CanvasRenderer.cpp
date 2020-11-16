#include "stdafx.h"
#include "CanvasRenderer.h"


namespace XYZ {
	CanvasRenderer::CanvasRenderer(
		Ref<XYZ::Material>     material,
		Ref<SubTexture2D>      subTexture,
		glm::vec4		       color,
		const XYZ::Mesh&	   mesh,
		uint32_t			   sortLayer,
		bool			       isVisible
	)
		:
		Material(material),
		SubTexture(subTexture),
		Color(color),
		Mesh(mesh),
		SortLayer(sortLayer),
		IsVisible(isVisible)
	{
	}
	CanvasRenderer::CanvasRenderer(const CanvasRenderer& other)
		:
		Material(other.Material),
		SubTexture(other.SubTexture),
		Color(other.Color),
		Mesh(other.Mesh),
		SortLayer(other.SortLayer),
		IsVisible(other.IsVisible)
	{
	}
	CanvasRenderer::CanvasRenderer(CanvasRenderer&& other) noexcept
		:
		Material(other.Material),
		SubTexture(other.SubTexture),
		Color(other.Color),
		Mesh(other.Mesh),
		SortLayer(other.SortLayer),
		IsVisible(other.IsVisible)
	{
	}
	CanvasRenderer& CanvasRenderer::operator=(const CanvasRenderer& other)
	{
		Material = other.Material;
		SubTexture = other.SubTexture;
		Color = other.Color;
		Mesh = other.Mesh;
		SortLayer = other.SortLayer;
		IsVisible = other.IsVisible;

		return *this;
	}
}