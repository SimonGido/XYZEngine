#include "stdafx.h"
#include "Prefab.h"

#include "XYZ/Scene/Components.h"

namespace XYZ {
	template<typename T>
	static void CopyComponentIfExists(Entity dst, ECSManager& dstEcs, Entity src, ECSManager& srcEcs)
	{
		if (srcEcs.IsValid(src) && srcEcs.HasComponent<T>(src))
		{
			auto& srcComponent = srcEcs.GetComponent<T>(src);
			if (dstEcs.HasComponent<T>(dst))
			{
				dstEcs.GetComponent<T>(dst) = srcComponent;
			}
			else
			{
				dstEcs.AddComponent(dst, srcComponent);
			}
		}
	}

	static void CopyAllComponentsIfExist(Entity dst, ECSManager& dstEcs, Entity src, ECSManager& srcEcs)
	{
		CopyComponentIfExists<TransformComponent>(		  dst, dstEcs, src, srcEcs);
		CopyComponentIfExists<SceneTagComponent>(		  dst, dstEcs, src, srcEcs);
		CopyComponentIfExists<SpriteRenderer>(			  dst, dstEcs, src, srcEcs);
		CopyComponentIfExists<MeshComponent>(			  dst, dstEcs, src, srcEcs);
		CopyComponentIfExists<AnimatedMeshComponent>(	  dst, dstEcs, src, srcEcs);
		CopyComponentIfExists<ParticleRenderer>(		  dst, dstEcs, src, srcEcs);
		CopyComponentIfExists<CameraComponent>(			  dst, dstEcs, src, srcEcs);
		CopyComponentIfExists<ParticleComponent>(		  dst, dstEcs, src, srcEcs);
		CopyComponentIfExists<PointLight2D>(			  dst, dstEcs, src, srcEcs);
		CopyComponentIfExists<SpotLight2D>(				  dst, dstEcs, src, srcEcs);
		CopyComponentIfExists<ScriptComponent>(			  dst, dstEcs, src, srcEcs);
		CopyComponentIfExists<RigidBody2DComponent>(	  dst, dstEcs, src, srcEcs);
		CopyComponentIfExists<BoxCollider2DComponent>(	  dst, dstEcs, src, srcEcs);
		CopyComponentIfExists<CircleCollider2DComponent>( dst, dstEcs, src, srcEcs);
		CopyComponentIfExists<PolygonCollider2DComponent>(dst, dstEcs, src, srcEcs);
		CopyComponentIfExists<ChainCollider2DComponent>(  dst, dstEcs, src, srcEcs);
		// No Relationship and IDComponent
	}

	Prefab::Prefab()
	{
		m_Scene = Ref<Scene>::Create("Prefab");
		m_Entity = m_Scene->CreateEntity("Prefab", GUID());
	}

	void Prefab::Create(SceneEntity entity)
	{
		m_Scene = Ref<Scene>::Create("Prefab");
		m_Entity = createPrefabFromEntity(entity);
		if (m_Entity.HasComponent<AnimatedMeshComponent>())
		{
			auto& meshComponent = m_Entity.GetComponent<AnimatedMeshComponent>();
			if (meshComponent.Mesh.Raw() && meshComponent.Mesh->IsValid())
			{
				auto& skeleton = meshComponent.Mesh->GetMeshSource();
			}
		}
	}
	SceneEntity Prefab::Instantiate(Ref<Scene> dstScene, SceneEntity parent, const glm::vec3* translation, const glm::vec3* rotation, const glm::vec3* scale)
	{
		SceneEntity newEntity;
		if (parent)
			newEntity = dstScene->CreateEntity(m_Entity.GetComponent<SceneTagComponent>(), parent, GUID());
		else
			newEntity = dstScene->CreateEntity(m_Entity.GetComponent<SceneTagComponent>(), GUID());

		copyEntity(newEntity, m_Entity);

		auto& transformComponent = newEntity.GetComponent<TransformComponent>();
		if (translation)
			transformComponent.Translation = *translation;
		if (rotation)
			transformComponent.Rotation = *rotation;
		if (scale)
			transformComponent.Scale = *scale;

		return newEntity;
	}
	void Prefab::copyEntity(SceneEntity dst, SceneEntity src) const
	{
		CopyAllComponentsIfExist(dst, *dst.GetECS(), src, *src.GetECS());
		auto& srcRel = src.GetComponent<Relationship>();
		auto& dstRel = dst.GetComponent<Relationship>();
		std::stack<Entity> children;
		if (srcRel.GetFirstChild())
			children.push(srcRel.GetFirstChild());

		while (!children.empty())
		{
			SceneEntity child = SceneEntity(children.top(), src.GetScene());
			children.pop();

			auto& childRel = child.GetComponent<Relationship>();
			SceneEntity newChild = dst.GetScene()->CreateEntity(src.GetComponent<SceneTagComponent>().Name, dst, GUID());
			copyEntity(newChild, child);
		}
	}
	SceneEntity Prefab::createPrefabFromEntity(SceneEntity entity)
	{
		SceneEntity newEntity = m_Scene->CreateEntity("", GUID());
		CopyAllComponentsIfExist(newEntity, m_Scene->GetECS(), entity, *entity.GetECS());
	
		std::stack<Entity> children;
		auto& dstRel = newEntity.GetComponent<Relationship>();
		auto& srcRel = entity.GetComponent<Relationship>();

		if (dstRel.GetFirstChild())
			children.push(dstRel.GetFirstChild());
		
		while (!children.empty())
		{
			SceneEntity child(children.top(), entity.GetScene());
			children.pop();

			auto& childRel = child.GetComponent<Relationship>();
			SceneEntity newChild = createPrefabFromEntity(child);
			Relationship::SetupRelation(newEntity, newChild, m_Scene->GetECS());
			if (childRel.GetNextSibling())
				children.push(childRel.GetNextSibling());
		}

		return entity;
	}
}