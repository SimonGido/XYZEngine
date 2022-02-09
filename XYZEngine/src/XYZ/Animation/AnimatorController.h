#pragma once
#include "XYZ/Asset/Asset.h"
#include "XYZ/Scene/SceneEntity.h"
#include "Animation.h"


namespace XYZ {
	
	class Animator;
	class AnimatorController : public Asset
	{
	public:
		void Update(Ref<Animator>& animator);

	private:

	};

	class Animator : public RefCount
	{
	public:
		Animator();
		void Update(Timestep ts);
		void SetSceneEntity(const SceneEntity& entity);
		void AddAnimation(const std::string& name, const Ref<Animation>& animation);
		void CreateAvatars();
	private:
		struct AnimationData
		{
			Ref<Animation>	Animation;
			AnimationAvatar Avatar;
		};

		SceneEntity	   m_Entity;
		AnimationData* m_CurrentAnimation;

		std::unordered_map<std::string, AnimationData> m_Animations;

		friend AnimatorController;
	};
}