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

		template <typename T>
		void AddComponent(const T& component);

		static AssetType GetStaticType() { return AssetType::Prefab; }
		virtual AssetType GetAssetType() const override { return GetStaticType(); }
	private:
		SceneEntity createPrefabFromEntity(SceneEntity entity);
		void copyEntity(SceneEntity dst, SceneEntity src) const;
	private:
		Ref<Scene>  m_Scene;
		SceneEntity m_Entity;
	};
	template<typename T>
	inline void Prefab::AddComponent(const T& component)
	{
		if (!m_Entity.HasComponent<T>())
			m_Entity.AddComponent<T>(component);
	}
}