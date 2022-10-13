#include "stdafx.h"
#include "Prefab.h"

#include "Components.h"

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

		template<typename T>
		static void CopyComponentIfExists(const entt::registry& src, entt::registry& dst, entt::entity srcEntity, entt::entity dstEntity)
		{
			if (src.any_of<const T>(srcEntity))
			{
				const auto& srcComponent = src.get<const T>(srcEntity);

				dst.emplace_or_replace<T>(dstEntity, srcComponent);
			}
		}

		template <typename ...Args>
		static void CopyComponentsIfExist(const entt::registry& src, entt::registry& dst, entt::entity srcEntity, entt::entity dstEntity)
		{
			(CopyComponentIfExists<Args>(src, dst, srcEntity, dstEntity), ...);
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

	static void CopyComponentsIfExist(const entt::registry& src, entt::registry& dst, entt::entity srcEntity, entt::entity dstEntity)
	{
		IDComponent idComponentCopy = dst.get<IDComponent>(dstEntity);
		Relationship relationshipCopy = dst.get<Relationship>(dstEntity);

		Utils::CopyComponentsIfExist<XYZ_COMPONENTS>(src, dst, srcEntity, dstEntity);

		// We do not want to override theese components
		dst.get<IDComponent>(dstEntity) = idComponentCopy;
		dst.get<Relationship>(dstEntity) = relationshipCopy;

	}

	Prefab::Prefab()
	{
		m_Scene = Ref<Scene>::Create("Prefab");
		m_Entity = m_Scene->CreateEntity("First Entity");
	}
	Prefab::Prefab(SceneEntity entity)
	{
		Create(entity);
	}
	Prefab::Prefab(const Ref<AnimatedMesh>& mesh, std::string name)
	{
		Create(mesh, name);
	}
	void Prefab::Create(SceneEntity entity)
	{
		m_Scene = Ref<Scene>::Create("Prefab");
		m_Entity = createPrefabFromEntity(entity);
		
		// Find in hierarchy animated mesh component and assign it's bones
		for (auto& entity : m_Entities)
		{
			if (entity.HasComponent<AnimatedMeshComponent>())
				setupBoneEntities(entity);
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
		for (auto& entity : m_Entities)
		{
			if (entity.HasComponent<AnimatedMeshComponent>())
				setupBoneEntities(entity);
		}
	}

	SceneEntity Prefab::Instantiate(Ref<Scene> dstScene, SceneEntity parent, const glm::vec3* translation, const glm::vec3* rotation, const glm::vec3* scale)
	{
		SceneEntity newEntity;
		if (parent.IsValid())
			newEntity = dstScene->CreateEntity(m_Entity.GetComponent<SceneTagComponent>(), parent, GUID());
		else
			newEntity = dstScene->CreateEntity(m_Entity.GetComponent<SceneTagComponent>(), GUID());
		newEntity.EmplaceComponent<PrefabComponent>(Ref<Prefab>(this), newEntity.ID());

		std::unordered_map<entt::entity, entt::entity> clones;
		copyEntity(newEntity, m_Entity, clones);
		
		for (auto& prefabEntity : m_Entities)
		{
			const entt::entity clone = clones[prefabEntity.ID()];
			SceneEntity cloneEntity(clone, dstScene.Raw());
			if (prefabEntity.HasComponent<AnimatedMeshComponent>())
			{
				auto& animMeshComponent = cloneEntity.GetComponent<AnimatedMeshComponent>();
				// Remap prefab entity to created clone entity
				for (auto& boneEntity : animMeshComponent.BoneEntities)
					boneEntity = clones[boneEntity];
			}
		}

		auto& transformComponent = newEntity.GetComponent<TransformComponent>();
		if (translation)
			transformComponent.GetTransform().Translation = *translation;
		if (rotation)
			transformComponent.GetTransform().Rotation = *rotation;
		if (scale)
			transformComponent.GetTransform().Scale = *scale;

		return newEntity;
	}




	void Prefab::copyEntity(SceneEntity dst, SceneEntity src, std::unordered_map<entt::entity, entt::entity>& clones) const
	{
		CopyComponentsIfExist(*src.GetRegistry(), *dst.GetRegistry(), src.ID(), dst.ID());

		clones[src.ID()] = dst.ID();

		auto& srcRel = src.GetComponent<Relationship>();
		auto& dstRel = dst.GetComponent<Relationship>();
		std::stack<entt::entity> children;
		if (srcRel.GetFirstChild() != entt::null)
			children.push(srcRel.GetFirstChild());

		while (!children.empty())
		{
			SceneEntity child = SceneEntity(children.top(), src.GetScene());
			children.pop();

			auto& childRel = child.GetComponent<Relationship>();
			SceneEntity newChild = dst.GetScene()->CreateEntity(src.GetComponent<SceneTagComponent>().Name, dst, GUID());
			copyEntity(newChild, child, clones);
			if (childRel.GetNextSibling() != entt::null)
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
			auto it = bones.find(entity.GetComponent<SceneTagComponent>().Name);
			if (it != bones.end())
			{
				animatedMeshComponent.BoneEntities[it->second] = entity.ID();
			}
		}
	}
	SceneEntity Prefab::createPrefabFromEntity(SceneEntity entity)
	{
		SceneEntity newEntity = m_Scene->CreateEntity("", GUID());
		m_Entities.push_back(newEntity);

		CopyComponentsIfExist(*entity.GetRegistry(), m_Scene->GetRegistry(), entity.ID(), newEntity.ID());

		std::stack<entt::entity> children;
		
		auto& srcRel = entity.GetComponent<Relationship>();

		if (srcRel.GetFirstChild() != entt::null)
			children.push(srcRel.GetFirstChild());
		
		while (!children.empty())
		{
			SceneEntity child(children.top(), entity.GetScene());
			children.pop();

			auto& childRel = child.GetComponent<Relationship>();
			SceneEntity newChild = createPrefabFromEntity(child);
			Relationship::SetupRelation(newEntity.ID(), newChild.ID(), m_Scene->GetRegistry());
			if (childRel.GetNextSibling() != entt::null)
				children.push(childRel.GetNextSibling());
		}

		return newEntity;
	}
}