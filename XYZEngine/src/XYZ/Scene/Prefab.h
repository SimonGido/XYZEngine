#pragma once
#include "XYZ/Asset/Asset.h"
#include "SceneEntity.h"
#include "Scene.h"

namespace XYZ {
	class Prefab : public Asset
	{
	public:
		Prefab();
		void Create(SceneEntity entity);
		void Create(const Ref<AnimatedMesh>& mesh, std::string name = "");
		
		SceneEntity Instantiate(Ref<Scene> dstScene, SceneEntity parent = SceneEntity(), 
			const glm::vec3* translation = nullptr, const glm::vec3* rotation = nullptr, const glm::vec3* scale = nullptr);


		static AssetType GetStaticType() { return AssetType::Prefab; }
		virtual AssetType GetAssetType() const override { return GetStaticType(); }


		const std::vector<SceneEntity>& GetEntities() const { return m_Entities; }
	private:
		SceneEntity createPrefabFromEntity(SceneEntity entity);
		void copyEntity(SceneEntity dst, SceneEntity src, std::unordered_map<entt::entity, entt::entity>& clones) const;
		void setupBoneEntities(SceneEntity entity);

	private:
		Ref<Scene>  m_Scene;
		SceneEntity m_Entity;

		std::vector<SceneEntity> m_Entities;
	};
}