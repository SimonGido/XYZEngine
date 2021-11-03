#pragma once
#include "stdafx.h"
#include "Components.h"

#define GLM_ENABLE_EXPERIMENTAL
#include <glm/gtx/quaternion.hpp>

namespace XYZ {

	SpriteRenderer::SpriteRenderer(const XYZ::Ref<XYZ::Material>& material, const Ref<XYZ::SubTexture>& subTexture, const glm::vec4& color, uint32_t sortLayer, bool isVisible)
		:
		Material(material),
		SubTexture(subTexture),
		Color(color),
		SortLayer(sortLayer),
		Visible(isVisible)
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

	Entity Relationship::FindByName(const ECSManager& ecs, std::string_view name) const
	{
		std::stack<Entity> temp;
		temp.push(FirstChild);
		while (!temp.empty())
		{
			Entity entity = temp.top();
			temp.pop();

			if (ecs.GetComponent<SceneTagComponent>(entity).Name == name)
					return entity;

			const auto& relationship = ecs.GetComponent<Relationship>(entity);
			if (relationship.NextSibling)
				temp.push(relationship.NextSibling);
			if (relationship.FirstChild)
				temp.push(relationship.FirstChild);
		}
		return Entity();
	}

	std::vector<Entity> Relationship::GetTree(const ECSManager& ecs) const
	{
		std::vector<Entity> result;
		std::stack<Entity> temp;
		temp.push(FirstChild);
		while (!temp.empty())
		{
			Entity entity = temp.top();
			temp.pop();
			
			const auto& relationship = ecs.GetComponent<Relationship>(entity);
			if (relationship.NextSibling)
			{
				temp.push(relationship.NextSibling);
				result.push_back(relationship.NextSibling);
			}
			if (relationship.FirstChild)
			{
				temp.push(relationship.FirstChild);
				result.push_back(relationship.FirstChild);
			}
		}
		return result;
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
		glm::vec3 euler = glm::eulerAngles(rot);
		return std::tuple<glm::vec3, glm::vec3, glm::vec3>(translation, euler, scale);
	}
	glm::mat4 TransformComponent::GetTransform() const
	{
		glm::mat4 rotation = glm::toMat4(glm::quat(Rotation));
		return glm::translate(glm::mat4(1.0f), Translation)
			* rotation
			* glm::scale(glm::mat4(1.0f), Scale);
	}
	void TransformComponent::DecomposeTransform(const glm::mat4& transform)
	{
		glm::quat rotation;
		glm::vec3 skew;
		glm::vec4 perspective;

		glm::decompose(transform, Scale, rotation, Translation, skew, perspective);
		Rotation = glm::eulerAngles(rotation);
	}
}