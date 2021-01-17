#pragma once
#include "stdafx.h"
#include "Components.h"



namespace XYZ {
	SpriteRenderer::SpriteRenderer(XYZ::Ref<XYZ::Material> material, Ref<XYZ::SubTexture> subTexture, const glm::vec4& color, uint32_t sortLayer, bool isVisible)
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

	void Relationship::SetupRelation(uint32_t parent, uint32_t child, ECSManager& ecs)
	{
		auto& parentRel = ecs.GetComponent<Relationship>(parent);
		auto& childRel = ecs.GetComponent<Relationship>(child);

		childRel.Parent = parent;
		if (parentRel.FirstChild != NULL_ENTITY)
		{
			// Parent has first child
			auto& parentFirstChildRel = ecs.GetComponent<Relationship>(parentRel.FirstChild);
			// Set new child of parent as previous sibling of first child
			parentFirstChildRel.PreviousSibling = child;
			childRel.NextSibling = parentRel.FirstChild;
			parentRel.FirstChild = child;
		}
		else
		{
			parentRel.FirstChild = child;
		}
	}

	void Relationship::RemoveRelation(uint32_t child, ECSManager& ecs)
	{
		auto& childRel =  ecs.GetComponent<Relationship>(child);
		auto& parentRel = ecs.GetComponent<Relationship>(childRel.Parent);

		// No siblings ( must be first child of parent )
		if (childRel.NextSibling == NULL_ENTITY && childRel.PreviousSibling == NULL_ENTITY)
		{
			childRel.Parent = NULL_ENTITY;
			parentRel.FirstChild = NULL_ENTITY;
		}

		uint32_t current = parentRel.FirstChild;
		if (child == current) // If child is first child of parent
		{
			auto& currentRel = ecs.GetComponent<Relationship>(current);
			if (currentRel.NextSibling != NULL_ENTITY)
			{
				auto& nextRel = ecs.GetComponent<Relationship>(currentRel.NextSibling);
				nextRel.PreviousSibling = NULL_ENTITY;
				parentRel.FirstChild = currentRel.NextSibling;
			}
		}
		else // Else find where it is and remove from hierarchy
		{
			while (current != NULL_ENTITY)
			{
				auto& currentRel = ecs.GetComponent<Relationship>(current);
				if (current == child)
				{
					if (currentRel.PreviousSibling != NULL_ENTITY)
					{
						auto& prevRel = ecs.GetComponent<Relationship>(currentRel.PreviousSibling);
						prevRel.NextSibling = currentRel.NextSibling;
					}
					if (currentRel.NextSibling != NULL_ENTITY)
					{
						auto& nextRel = ecs.GetComponent<Relationship>(currentRel.NextSibling);
						nextRel.PreviousSibling = currentRel.PreviousSibling;
					}
					return;
				}
				current = currentRel.NextSibling;
			}
		}
	}
}