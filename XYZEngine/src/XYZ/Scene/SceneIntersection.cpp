#include "stdafx.h"
#include "SceneIntersection.h"
#include "SceneEntityUtils.h"

#include "Components.h"

namespace XYZ {
	namespace Utils {
		
		static bool RayEntityCollision(const SceneEntity& entity, const Ray& ray, float& distance)
		{
			AABB aabb = SceneEntityAABB(entity);
			if (ray.IntersectsAABB(aabb, distance))
			{
				if (entity.HasComponent<AnimatedMeshComponent>())
				{
					auto& meshComponent = entity.GetComponent<AnimatedMeshComponent>();
					auto& transformComponent = entity.GetComponent<TransformComponent>();

					Ray transformedRay = {
						glm::inverse(transformComponent.WorldTransform) * glm::vec4(ray.Origin, 1.0f),
						glm::inverse(glm::mat3(transformComponent.WorldTransform)) * ray.Direction
					};

					if (meshComponent.Mesh->IsValid())
					{
						auto& meshSource = meshComponent.Mesh->GetMeshSource();
						for (const auto& tr : meshSource->GetTriangles())
						{
							if (transformedRay.IntersectsTriangle(tr.V0, tr.V1, tr.V2, distance))
							{
								return true;
							}
						}
					}
				}
				else if (entity.HasComponent<MeshComponent>())
				{
					auto& meshComponent = entity.GetComponent<MeshComponent>();
					auto& transformComponent = entity.GetComponent<TransformComponent>();

					Ray transformedRay = {
						glm::inverse(transformComponent.WorldTransform) * glm::vec4(ray.Origin, 1.0f),
						glm::inverse(glm::mat3(transformComponent.WorldTransform)) * ray.Direction
					};

					if (meshComponent.Mesh->IsValid())
					{
						auto& meshSource = meshComponent.Mesh->GetMeshSource();
						for (const auto& tr : meshSource->GetTriangles())
						{
							if (transformedRay.IntersectsTriangle(tr.V0, tr.V1, tr.V2, distance))
							{
								return true;
							}
						}
					}
				}
				else if (entity.HasComponent<SpriteRenderer>())
				{
					return true;
				}
			}
			return false;
		}
	}


    std::deque<SceneIntersection::HitData> SceneIntersection::Intersect(const Ray& ray, Ref<Scene> scene)
    {
        std::deque<HitData> result;

		// TODO: octree
		scene->GetRegistry().each([&](const entt::entity entityID) {
			SceneEntity entity(entityID, scene.Raw());
			float distance = 0.0f;

			if (Utils::RayEntityCollision(entity, ray, distance))
			{
				result.push_back({ entity, distance });
			}
		});

		std::sort(result.begin(), result.end(), [&](const HitData& a, const HitData& b) {
			return a.Distance < b.Distance;
		});

        return result;
    }
}