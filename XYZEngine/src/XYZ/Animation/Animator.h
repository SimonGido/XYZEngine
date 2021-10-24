#pragma once
#include "XYZ/Asset/Asset.h"
#include "XYZ/Scene/SceneEntity.h"
#include "Animation.h"


namespace XYZ {
	class Animator : public Asset
	{
	public:
		void Update(Timestep ts);
		void SetAnimation(const Ref<Animation>& animation);
		void SetSceneEntity(const SceneEntity& entity);
		void UpdateAnimationEntities();

		SceneEntity			  GetSceneEntity() { return m_Entity; }
		const SceneEntity&	  GetSceneEntity() const { return m_Entity; }
		const Ref<Animation>& GetAnimation() const { return m_Animation;}

	private:
		void buildEntityPropertyPairs();

	private:
		Ref<Animation> m_Animation;
		SceneEntity	   m_Entity;

		using EntityPropertyPair = std::pair<SceneEntity, IProperty*>;

		std::vector<EntityPropertyPair> m_Properties;
	};
}