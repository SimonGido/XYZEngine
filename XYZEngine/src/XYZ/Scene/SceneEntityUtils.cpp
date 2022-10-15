#include "stdafx.h"
#include "SceneEntityUtils.h"

#include "Components.h"

namespace XYZ {
	AABB SceneEntityAABB(const SceneEntity& entity)
	{
		if (entity.HasComponent<AnimatedMeshComponent>())
		{
			auto& meshComponent = entity.GetComponent<AnimatedMeshComponent>();
			if (meshComponent.Mesh->IsValid())
			{
				Ref<MeshSource> meshSource = meshComponent.Mesh->GetMeshSource();
				AABB aabb = meshSource->GetSubmeshBoundingBox();

				const Relationship& relationship = entity.GetComponent<Relationship>();
				const TransformComponent& parentTransformComponent = entity.GetRegistry()->get<TransformComponent>(relationship.GetParent());
				aabb.Min = glm::vec3(parentTransformComponent->WorldTransform * meshSource->GetSubmeshTransform() * glm::vec4(aabb.Min, 1.0f));
				aabb.Max = glm::vec3(parentTransformComponent->WorldTransform * meshSource->GetSubmeshTransform() * glm::vec4(aabb.Max, 1.0f));
				return aabb;
			}
		}
		if (entity.HasComponent<MeshComponent>())
		{
			auto& meshComponent = entity.GetComponent<MeshComponent>();
			if (meshComponent.Mesh->IsValid())
			{
				const TransformComponent& transformComponent = entity.GetComponent<TransformComponent>();
				AABB aabb = meshComponent.Mesh->GetMeshSource()->GetSubmeshBoundingBox();
				aabb.Min = glm::vec3(transformComponent->WorldTransform * glm::vec4(aabb.Min, 1.0f));
				aabb.Max = glm::vec3(transformComponent->WorldTransform * glm::vec4(aabb.Max, 1.0f));
				return aabb;
			}
		}
		else if (entity.HasComponent<SpriteRenderer>())
		{
			const TransformComponent& transformComponent = entity.GetComponent<TransformComponent>();
			auto [translation, rotation, scale] = transformComponent.GetWorldComponents();
			return AABB(
				translation - (scale / 2.0f),
				translation + (scale / 2.0f)
			);
		}
		return AABB();
	}
}