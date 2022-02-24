#include "stdafx.h"
#include "Prefab.h"

#include "XYZ/Scene/Components.h"

#include "XYZ/Asset/AssetManager.h"

#include <assimp/scene.h>

namespace XYZ {

	namespace Utils {
		static glm::mat4 Mat4FromAIMatrix4x4(const aiMatrix4x4& matrix)
		{
			glm::mat4 result;
			//the a,b,c,d in assimp is the row ; the 1,2,3,4 is the column
			result[0][0] = matrix.a1; result[1][0] = matrix.a2; result[2][0] = matrix.a3; result[3][0] = matrix.a4;
			result[0][1] = matrix.b1; result[1][1] = matrix.b2; result[2][1] = matrix.b3; result[3][1] = matrix.b4;
			result[0][2] = matrix.c1; result[1][2] = matrix.c2; result[2][2] = matrix.c3; result[3][2] = matrix.c4;
			result[0][3] = matrix.d1; result[1][3] = matrix.d2; result[2][3] = matrix.d3; result[3][3] = matrix.d4;
			return result;
		}
	}

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


	static void BuildMeshEntityHierarchy(SceneEntity parent, Ref<Scene>& scene, const Ref<AnimatedMesh>& mesh, aiNode* node, std::vector<SceneEntity>& entities)
	{
		XYZ_ASSERT(node->mNumMeshes <= 1, "We support only one mesh per assimp scene now");
		glm::mat4 transform = Utils::Mat4FromAIMatrix4x4(node->mTransformation);
		auto nodeName = node->mName.C_Str();

		SceneEntity childEntity;
		if (parent)
			childEntity = scene->CreateEntity(nodeName, parent);
		else
			childEntity = scene->CreateEntity(nodeName);

		entities.push_back(childEntity);


		childEntity.GetComponent<TransformComponent>().DecomposeTransform(transform);
		if (node->mNumMeshes == 1)
		{
			Ref<MaterialAsset> materialAsset = AssetManager::GetAsset<MaterialAsset>("Resources/Materials/AnimMeshMaterial.mat");
			auto& mc = childEntity.EmplaceComponent<AnimatedMeshComponent>(mesh, materialAsset);
		}

		for (uint32_t i = 0; i < node->mNumChildren; i++)
			BuildMeshEntityHierarchy(childEntity, scene, mesh, node->mChildren[i], entities);
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
	void Prefab::Create(const Ref<AnimatedMesh>& mesh, std::string name)
	{		
		if (name.empty())
		{
			const auto& metadata = AssetManager::GetMetadata(mesh);
			name = metadata.FilePath.stem().string();
		}
		m_Scene = Ref<Scene>::Create("Prefab");
		m_Entity = m_Scene->CreateEntity(name);

		auto assimpScene = mesh->GetMeshSource()->GetScene();
		if (assimpScene->mRootNode->mNumMeshes == 0)
		{
			for (uint32_t i = 0; i < assimpScene->mRootNode->mNumChildren; i++)
				BuildMeshEntityHierarchy(m_Entity, m_Scene, mesh, assimpScene->mRootNode->mChildren[i], m_Entities);
		}
		else
		{
			BuildMeshEntityHierarchy(m_Entity, m_Scene, mesh, assimpScene->mRootNode, m_Entities);
		}

		// Find in hierarchy animated mesh component and assign it's bones
		for (auto &entity : m_Entities)
		{
			if (entity.HasComponent<AnimatedMeshComponent>())
				setupBoneEntities(entity);
		}
	}
	SceneEntity Prefab::Instantiate(Ref<Scene> dstScene, SceneEntity parent, const glm::vec3* translation, const glm::vec3* rotation, const glm::vec3* scale)
	{
		SceneEntity newEntity;
		if (parent)
			newEntity = dstScene->CreateEntity(m_Entity.GetComponent<SceneTagComponent>(), parent, GUID());
		else
			newEntity = dstScene->CreateEntity(m_Entity.GetComponent<SceneTagComponent>(), GUID());
		newEntity.EmplaceComponent<PrefabComponent>(Ref<Prefab>(this), newEntity.ID());

		std::unordered_map<Entity, Entity> clones;
		copyEntity(newEntity, m_Entity, clones);
		
		for (auto& prefabEntity : m_Entities)
		{
			const Entity clone = clones[prefabEntity.ID()];
			SceneEntity cloneEntity(clone, dstScene.Raw());
			if (prefabEntity.HasComponent<AnimatedMeshComponent>())
			{
				auto& animMeshComponent = cloneEntity.GetComponent<AnimatedMeshComponent>();
				// Remap prefab entity to created clone entity
				for (auto& boneEntity : animMeshComponent.BoneEntities)
					boneEntity = clones[boneEntity];
			}
			if (prefabEntity.HasComponent<AnimationComponent>())
			{
				
				auto& animComponent = cloneEntity.GetComponent<AnimationComponent>();
				// Remap prefab entity to created clone entity
				for (auto& boneEntity : animComponent.BoneEntities)
					boneEntity = clones[boneEntity];
			}
		}

		auto& transformComponent = newEntity.GetComponent<TransformComponent>();
		if (translation)
			transformComponent.Translation = *translation;
		if (rotation)
			transformComponent.Rotation = *rotation;
		if (scale)
			transformComponent.Scale = *scale;

		return newEntity;
	}
	void Prefab::copyEntity(SceneEntity dst, SceneEntity src, std::unordered_map<Entity, Entity>& clones) const
	{
		CopyAllComponentsIfExist(dst, *dst.GetECS(), src, *src.GetECS());
		clones[src] = dst;

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
			copyEntity(newChild, child, clones);
			if (childRel.GetNextSibling())
				children.push(childRel.GetNextSibling());
		}
	}
	void Prefab::setupBoneEntities(SceneEntity entity)
	{
		auto& animatedMeshComponent = entity.GetComponent<AnimatedMeshComponent>();
		auto& bones = animatedMeshComponent.Mesh->GetMeshSource()->GetBoneMapping();
		auto& boneInfo = animatedMeshComponent.Mesh->GetMeshSource()->GetBoneInfo();
		animatedMeshComponent.BoneEntities.resize(boneInfo.size());
		for (const auto& entity : m_Entities)
		{
			for (const auto& [name, index] : bones)
			{
				if (entity.GetComponent<SceneTagComponent>().Name == name)
				{
					animatedMeshComponent.BoneEntities[index] = entity;
					break;
				}
			}
		}
	}
	SceneEntity Prefab::createPrefabFromEntity(SceneEntity entity)
	{
		SceneEntity newEntity = m_Scene->CreateEntity("", GUID());
		m_Entities.push_back(newEntity);
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