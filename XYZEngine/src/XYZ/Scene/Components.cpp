#pragma once
#include "stdafx.h"
#include "Components.h"


namespace XYZ {

	SpriteRenderer::SpriteRenderer(const XYZ::Ref<XYZ::Material>& material, const Ref<XYZ::SubTexture>& subTexture, const glm::vec4& color, uint32_t sortLayer, bool isVisible)
		:
		Material(material),
		SubTexture(subTexture),
		Color(color),
		SortLayer(sortLayer),
		Visible(Visible)
	{}

	SpriteRenderer::SpriteRenderer(const XYZ::SpriteRenderer& other)
		:
		Material(other.Material),
		SubTexture(other.SubTexture),
		Color(other.Color),
		SortLayer(other.SortLayer),
		Visible(other.Visible)
	{
	}

	SpriteRenderer::SpriteRenderer(XYZ::SpriteRenderer&& other) noexcept
		:
		Material(other.Material),
		SubTexture(other.SubTexture),
		Color(other.Color),
		SortLayer(other.SortLayer),
		Visible(other.Visible)
	{
	}

	SpriteRenderer& XYZ::SpriteRenderer::operator=(const XYZ::SpriteRenderer& other)
	{
		Material = other.Material;
		SubTexture = other.SubTexture;
		Color = other.Color;
		SortLayer = other.SortLayer;
		Visible = other.Visible;

		return *this;
	}

	Relationship::Relationship()
		: 
		Depth(0)
	{
	}

	Relationship::Relationship(Entity parent)
		:
		Parent(parent),
		Depth(0)
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

		childRel.Depth = parentRel.Depth + 1;
	}

	void Relationship::RemoveRelation(Entity child, ECSManager& ecs)
	{
		removeRelation(child, ecs);
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
			childRel.Depth = 0;
		}
	}
	std::tuple<glm::vec3, glm::vec3, glm::vec3> TransformComponent::GetWorldComponents() const
	{
		glm::vec3 translation;
		glm::vec3 scale;
		glm::quat rot;
		glm::vec3 skew;
		glm::vec4 perspective;
		glm::decompose(WorldTransform, scale, rot, translation, skew, perspective);
		glm::vec3 euler = glm::eulerAngles(rot) * glm::pi<float>() / 180.0f;
		return std::tuple<glm::vec3, glm::vec3, glm::vec3>(translation, euler, scale);
	}
	glm::mat4 TransformComponent::GetTransform() const
	{
		glm::mat4 rotation = glm::rotate(glm::mat4(1.0f), Rotation.x, { 1, 0, 0 })
			* glm::rotate(glm::mat4(1.0f), Rotation.y, { 0, 1, 0 })
			* glm::rotate(glm::mat4(1.0f), Rotation.z, { 0, 0, 1 });

		return glm::translate(glm::mat4(1.0f), Translation)
			* rotation
			* glm::scale(glm::mat4(1.0f), Scale);
	}
}