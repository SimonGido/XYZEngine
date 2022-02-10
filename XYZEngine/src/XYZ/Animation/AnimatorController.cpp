#include "stdafx.h"
#include "AnimatorController.h"

#include "XYZ/Debug/Profiler.h"
#include "AnimatorController.h"

namespace XYZ {
	Animator::Animator()
		:
		m_CurrentAnimation(nullptr)
	{
	}
	void Animator::Update(Timestep ts)
	{
		XYZ_PROFILE_FUNC("Animator::Update");
		if (m_CurrentAnimation)
		{
			m_CurrentAnimation->Player.Update(ts);
		}
	}

	void Animator::SetSceneEntity(const SceneEntity& entity)
	{
		m_Entity = entity;
		CreatePlayers();
	}

	void Animator::AddAnimation(const std::string& name, const Ref<Animation>& animation)
	{
		XYZ_ASSERT(animation.Raw() && m_Entity.IsValid(), "");
		auto it = m_Animations.find(name);
		if (it == m_Animations.end())
		{
			AnimationPlayer avatar;
			if (!avatar.Create(m_Entity, animation))
				XYZ_WARN("Failed to create avatar for animation {} ", name);
			m_Animations[name] = { animation, avatar };
		}
		else
		{
			XYZ_WARN("Animation with name {} already exists", name);
		}
	}


	void Animator::CreatePlayers()
	{
		for (auto& [name, data] : m_Animations)
		{
			if (!data.Player.Create(m_Entity, data.Animation))
			{
				XYZ_WARN("Failed to create player for animation {} ", name);
			}
		}
	}

	void AnimatorController::Update(Ref<Animator>& animator)
	{
		// TODO:
	}

}