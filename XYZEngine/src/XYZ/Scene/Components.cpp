#pragma once
#include "stdafx.h"
#include "Components.h"



namespace XYZ {
	std::function<void(Entity entity, ECSManager& ecs)> Relationship::OnParentChanged;

	SpriteRenderer::SpriteRenderer(const XYZ::Ref<XYZ::Material>& material, const Ref<XYZ::SubTexture>& subTexture, const glm::vec4& color, uint32_t sortLayer, bool isVisible)
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

	Relationship::Relationship(Entity parent)
		:
		Parent(parent)
	{
	}

	void Relationship::SetupRelation(Entity parent, Entity child, ECSManager& ecs)
	{
		auto& parentRel = ecs.GetComponent<Relationship>(parent);
		auto& childRel = ecs.GetComponent<Relationship>(child);

		childRel.Parent = parent;
		if ((bool)parentRel.FirstChild)
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
		if (OnParentChanged)
			OnParentChanged(child, ecs);
	}

	void Relationship::RemoveRelation(Entity child, ECSManager& ecs)
	{
		auto& childRel =  ecs.GetComponent<Relationship>(child);
		auto& parentRel = ecs.GetComponent<Relationship>(childRel.Parent);

		// No siblings ( must be first child of parent )
		if (!(bool)childRel.NextSibling && !(bool)childRel.PreviousSibling)
		{
			childRel.Parent = Entity();
			parentRel.FirstChild = Entity();
		}

		Entity current = parentRel.FirstChild;
		if (child == current) // If child is first child of parent
		{
			auto& currentRel = ecs.GetComponent<Relationship>(current);
			if ((bool)currentRel.NextSibling)
			{
				auto& nextRel = ecs.GetComponent<Relationship>(currentRel.NextSibling);
				nextRel.PreviousSibling = Entity();
				parentRel.FirstChild = currentRel.NextSibling;
			}
		}
		else // Else find where it is and remove from hierarchy
		{
			while ((bool)current)
			{
				auto& currentRel = ecs.GetComponent<Relationship>(current);
				if (current == child)
				{
					if ((bool)currentRel.PreviousSibling)
					{
						auto& prevRel = ecs.GetComponent<Relationship>(currentRel.PreviousSibling);
						prevRel.NextSibling = currentRel.NextSibling;
					}
					if ((bool)currentRel.NextSibling)
					{
						auto& nextRel = ecs.GetComponent<Relationship>(currentRel.NextSibling);
						nextRel.PreviousSibling = currentRel.PreviousSibling;
					}
					break;
				}
				current = currentRel.NextSibling;
			}
		}
		childRel.Parent = Entity();
		if (OnParentChanged)
			OnParentChanged(child, ecs);
	}
}