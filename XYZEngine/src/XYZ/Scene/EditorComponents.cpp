#include "stdafx.h"
#include "EditorComponents.h"

namespace XYZ {
	EditorSpriteRenderer::EditorSpriteRenderer(const Ref<XYZ::Material>& material, const Ref<XYZ::SubTexture>& subTexture, const glm::vec4& color, bool isVisible)
		:
		Material(material),
		SubTexture(subTexture),
		Color(color)
	{
	}
	EditorSpriteRenderer::EditorSpriteRenderer(const EditorSpriteRenderer& other)
		:
		Material(other.Material),
		SubTexture(other.SubTexture),
		Color(other.Color)
	{
	}
	EditorSpriteRenderer::EditorSpriteRenderer(EditorSpriteRenderer&& other) noexcept
		:
		Material(std::move(other.Material)),
		SubTexture(std::move(other.SubTexture)),
		Color(other.Color)
	{
	}
	EditorSpriteRenderer& EditorSpriteRenderer::operator=(const EditorSpriteRenderer& other)
	{
		Material = other.Material;
		SubTexture = other.SubTexture;
		Color = other.Color;
		return *this;
	}
}