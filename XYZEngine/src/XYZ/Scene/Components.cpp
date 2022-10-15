#pragma once
#include "stdafx.h"
#include "Components.h"

#include "Prefab.h"

#include "XYZ/Debug/Profiler.h"

#define GLM_ENABLE_EXPERIMENTAL
#include <glm/gtx/quaternion.hpp>

namespace XYZ {

	SpriteRenderer::SpriteRenderer(const XYZ::Ref<XYZ::MaterialAsset>& material, const Ref<XYZ::SubTexture>& subTexture, const glm::vec4& color, uint32_t sortLayer, bool isVisible)
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
		Parent(entt::null),
		FirstChild(entt::null),
		PreviousSibling(entt::null),
		NextSibling(entt::null),
		Depth(0)
	
	{
	}

	Relationship::Relationship(const Relationship& other)
		:
		Parent(other.Parent),
		FirstChild(other.FirstChild),
		PreviousSibling(other.PreviousSibling),
		NextSibling(other.NextSibling),
		Depth(other.Depth)
	{
	}


	entt::entity Relationship::FindByName(const entt::registry& reg, std::string_view name) const
	{
		std::stack<entt::entity> temp;
		if (reg.valid(FirstChild))
			temp.push(FirstChild);

		while (!temp.empty())
		{
			entt::entity entity = temp.top();
			temp.pop();

			if (reg.get<SceneTagComponent>(entity).Name == name)
				return entity;

			const auto& relationship = reg.get<Relationship>(entity);
			if (reg.valid(relationship.NextSibling))
				temp.push(relationship.NextSibling);
			if (reg.valid(relationship.FirstChild))
				temp.push(relationship.FirstChild);
		}
		return entt::entity();
	}


	std::vector<entt::entity> Relationship::GetTree(const entt::registry& reg) const
	{
		std::vector<entt::entity> result;
		std::stack<entt::entity> temp;
		if (reg.valid(FirstChild))
			temp.push(FirstChild);
		while (!temp.empty())
		{
			const entt::entity entity = temp.top();
			result.push_back(entity);
			temp.pop();
			
			const auto& relationship = reg.get<Relationship>(entity);
			if (reg.valid(relationship.NextSibling))
				temp.push(relationship.NextSibling);
			
			if (reg.valid(relationship.FirstChild))
				temp.push(relationship.FirstChild);
		}
		return result;
	}


	
	bool Relationship::IsInHierarchy(const entt::registry& reg, entt::entity child) const
	{
		std::stack<entt::entity> temp;
		if (reg.valid(FirstChild))
			temp.push(FirstChild);
		while (!temp.empty())
		{
			const entt::entity entity = temp.top();
			temp.pop();

			if (entity == child)
				return true;

			const auto& relationship = reg.get<Relationship>(entity);
			if (reg.valid(relationship.NextSibling))
				temp.push(relationship.NextSibling);

			if (reg.valid(relationship.FirstChild))
				temp.push(relationship.FirstChild);
		}
		return false;
	}

	void Relationship::SetupRelation(entt::entity parent, entt::entity child, entt::registry& reg)
	{
		XYZ_PROFILE_FUNC("Relationship::SetupRelation");
		removeRelation(child, reg);
		Relationship& parentRel = reg.get<Relationship>(parent);
		Relationship& childRel = reg.get<Relationship>(child);
		Relationship* lastChildRel = nullptr;

		entt::entity lastChild = parentRel.FirstChild;
		while (reg.valid(lastChild)) // Find last child
		{
			lastChildRel = &reg.get<Relationship>(lastChild);
			if (!reg.valid(lastChildRel->NextSibling))
				break;
			lastChild = lastChildRel->NextSibling;
		}
		if (reg.valid(lastChild))
			lastChildRel->NextSibling = child;
		else
			parentRel.FirstChild = child;

		childRel.PreviousSibling = lastChild;
		childRel.Parent = parent;
		childRel.Depth = parentRel.Depth + 1;
	}

	void Relationship::RemoveRelation(entt::entity child, entt::registry& reg)
	{
		removeRelation(child, reg);
	}
	void Relationship::removeRelation(entt::entity child, entt::registry& reg)
	{
		XYZ_PROFILE_FUNC("Relationship::removeRelation");
		auto& childRel = reg.get<Relationship>(child);
		
		if (reg.valid(childRel.Parent))
		{
			auto& parentRel = reg.get<Relationship>(childRel.Parent);
			if (child == parentRel.FirstChild)
				parentRel.FirstChild = childRel.NextSibling;

			if (reg.valid(childRel.NextSibling))
			{
				auto& nextSiblingRel = reg.get<Relationship>(childRel.NextSibling);
				nextSiblingRel.PreviousSibling = childRel.PreviousSibling;
			}
			if (reg.valid(childRel.PreviousSibling))
			{
				auto& previousSiblingRel = reg.get<Relationship>(childRel.PreviousSibling);
				previousSiblingRel.NextSibling = childRel.NextSibling;
			}
			
			childRel.NextSibling = entt::null;
			childRel.PreviousSibling = entt::null;
			childRel.Parent = entt::null;
			childRel.Depth = 0;
		}
	}
	TransformComponent::TransformComponent(const TransformComponent& other)
		:
		m_Transform(other.m_Transform),
		m_Dirty(other.m_Dirty)
	{
	}
	TransformComponent::TransformComponent(const glm::vec3& translation)
	{
		m_Transform.Translation = translation;
	}
	std::tuple<glm::vec3, glm::vec3, glm::vec3> TransformComponent::GetWorldComponents() const
	{
		glm::vec3 translation;
		glm::vec3 scale;
		glm::quat rot;
		glm::vec3 skew;
		glm::vec4 perspective;
		glm::decompose(m_Transform.WorldTransform, scale, rot, translation, skew, perspective);
		glm::vec3 euler = glm::eulerAngles(rot);
		return std::tuple<glm::vec3, glm::vec3, glm::vec3>(translation, euler, scale);
	}
	glm::mat4 TransformComponent::GetLocalTransform() const
	{
		const glm::mat4 rotation = glm::toMat4(glm::quat(m_Transform.Rotation));
		return glm::translate(glm::mat4(1.0f), m_Transform.Translation)
			* rotation
			* glm::scale(glm::mat4(1.0f), m_Transform.Scale);
	}
	void TransformComponent::DecomposeTransform(const glm::mat4& transform)
	{
		glm::quat rotation;
		glm::vec3 skew;
		glm::vec4 perspective;

		glm::decompose(transform, m_Transform.Scale, rotation, m_Transform.Translation, skew, perspective);
		m_Transform.Rotation = glm::eulerAngles(rotation);
		m_Dirty = true;
	}
	
	MeshComponent::MeshComponent(const MeshComponent& other)
		:
		Mesh(other.Mesh),
		MaterialAsset(other.MaterialAsset),
		OverrideMaterial(other.OverrideMaterial)
	{
	}

	MeshComponent::MeshComponent(const Ref<XYZ::Mesh>& mesh, const Ref<XYZ::MaterialAsset>& materialAsset)
		:
		Mesh(mesh), MaterialAsset(materialAsset)
	{
	}

	ParticleRenderer::ParticleRenderer(const ParticleRenderer& other)
		:
		Mesh(other.Mesh),
		MaterialAsset(other.MaterialAsset),
		OverrideMaterial(other.OverrideMaterial)
	{
	}

	ParticleRenderer::ParticleRenderer(const Ref<XYZ::Mesh>& mesh, const Ref<XYZ::MaterialAsset>& materialAsset)
		:
		Mesh(mesh), MaterialAsset(materialAsset)
	{
	}
	AnimatedMeshComponent::AnimatedMeshComponent(const AnimatedMeshComponent& other)
		:
		Mesh(other.Mesh),
		MaterialAsset(other.MaterialAsset),
		OverrideMaterial(other.OverrideMaterial),
		BoneTransforms(other.BoneTransforms),
		BoneEntities(other.BoneEntities)
	{
	}
	AnimatedMeshComponent::AnimatedMeshComponent(const Ref<AnimatedMesh>& mesh, const Ref<XYZ::MaterialAsset>& materialAsset)
		:
		Mesh(mesh), MaterialAsset(materialAsset)
	{
	}
	
	PrefabComponent::PrefabComponent(const Ref<Prefab>& prefabAsset, const entt::entity owner)
		:
		PrefabAsset(prefabAsset),
		Owner(owner)
	{
	}
	PrefabComponent::PrefabComponent(const PrefabComponent& other)
		:
		PrefabAsset(other.PrefabAsset),
		Owner(other.Owner)
	{
	}
	CameraComponent::CameraComponent(const CameraComponent& other)
		:
		Camera(other.Camera)
	{
	}
	ParticleComponent::ParticleComponent()
	{
		m_System = Ref<ParticleSystem>::Create();
	}
	ParticleComponent::ParticleComponent(const ParticleComponent& other)
		:
		m_System(new ParticleSystem(*other.m_System))
	{
	}
	ParticleComponent& ParticleComponent::operator=(const ParticleComponent& other)
	{
		m_System = Ref<ParticleSystem>::Create(*other.m_System);
		return *this;
	}

	IDComponent::IDComponent(const IDComponent& other)
		:
		ID(other.ID)
	{
	}
	AnimationComponent::AnimationComponent(const AnimationComponent& other)
		:
		Controller(other.Controller),
		AnimationTime(other.AnimationTime),
		Playing(other.Playing)
	{
	}
	PointLightComponent2D::PointLightComponent2D(const glm::vec3& color, float radius, float intensity)
		:
		Color(color),
		Radius(radius),
		Intensity(intensity)
	{
	}
	PointLightComponent2D::PointLightComponent2D(const PointLightComponent2D& other)
		:
		Color(other.Color),
		Radius(other.Radius),
		Intensity(other.Intensity)
	{
	}
	SpotLightComponent2D::SpotLightComponent2D(const SpotLightComponent2D& other)
		:
		Color(other.Color),
		Radius(other.Radius),
		Intensity(other.Intensity),
		InnerAngle(other.InnerAngle),
		OuterAngle(other.OuterAngle)
	{
	}
	BoxCollider2DComponent::BoxCollider2DComponent(const BoxCollider2DComponent& other)
		:
		Size(other.Size),
		Offset(other.Offset),
		Density(other.Density),
		Friction(other.Friction),
		RuntimeFixture(other.RuntimeFixture)
	{
	}
	CircleCollider2DComponent::CircleCollider2DComponent(const CircleCollider2DComponent& other)
		:
		Offset(other.Offset),
		Radius(other.Radius),
		Density(other.Density),
		Friction(other.Friction),
		RuntimeFixture(other.RuntimeFixture)
	{
	}
	PolygonCollider2DComponent::PolygonCollider2DComponent(const PolygonCollider2DComponent& other)
		:
		Vertices(other.Vertices),
		Density(other.Density),
		Friction(other.Friction),
		RuntimeFixture(other.RuntimeFixture)
	{
	}
	ChainCollider2DComponent::ChainCollider2DComponent(const ChainCollider2DComponent& other)
		:
		Points(other.Points),
		Density(other.Density),
		Friction(other.Friction),
		InvertNormals(other.InvertNormals),
		RuntimeFixture(other.RuntimeFixture)
	{
	}
}