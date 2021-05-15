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
		removeRelation(child, ecs);
		auto& parentRel = ecs.GetComponent<Relationship>(parent);
		auto& childRel = ecs.GetComponent<Relationship>(child);

		if (parentRel.FirstChild)
		{
			auto& firstChildRel = ecs.GetComponent<Relationship>(parentRel.FirstChild);
			firstChildRel.PreviousSibling = child;
			childRel.NextSibling = parentRel.FirstChild;
		}
		childRel.Parent = parent;
		parentRel.FirstChild = child;

		if (OnParentChanged)
			OnParentChanged(child, ecs);
	}

	void Relationship::RemoveRelation(Entity child, ECSManager& ecs)
	{
		removeRelation(child, ecs);
		if (OnParentChanged)
			OnParentChanged(child, ecs);
	}
	void Relationship::removeRelation(Entity child, ECSManager& ecs)
	{
		auto& childRel = ecs.GetComponent<Relationship>(child);

		if (childRel.Parent)
		{
			auto& parentRel = ecs.GetComponent<Relationship>(childRel.Parent);
			if (child == parentRel.FirstChild)
				parentRel.FirstChild = childRel.NextSibling;

			if (childRel.NextSibling)
			{
				auto& nextSiblingRel = ecs.GetComponent<Relationship>(childRel.NextSibling);
				nextSiblingRel.PreviousSibling = childRel.PreviousSibling;
			}
			if (childRel.PreviousSibling)
			{
				auto& previousSiblingRel = ecs.GetComponent<Relationship>(childRel.PreviousSibling);
				previousSiblingRel.NextSibling = childRel.NextSibling;
			}
			childRel.NextSibling = Entity();
			childRel.PreviousSibling = Entity();
			childRel.Parent = Entity();		
		}
	}
}