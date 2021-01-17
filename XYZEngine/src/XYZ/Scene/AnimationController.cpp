#include "stdafx.h"
#include "AnimationController.h"


namespace XYZ {
	uint32_t AnimationController::AddAnimation(Ref<Animation> animation)
	{
		if (!m_CurrentAnimation)
			m_CurrentAnimation = animation;

		auto &state = m_StateMachine.CreateState();
		if (m_Animations.size() >= state.GetID())
			m_Animations.resize((size_t)state.GetID() + 1);
		m_Animations[state.GetID()] = animation;

		return state.GetID();
	}
	void AnimationController::Update(Timestep ts)
	{
		if (m_CurrentAnimation)
			m_CurrentAnimation->Update(ts);
	}

	void AnimationController::TransitionTo(uint32_t id)
	{
		if (m_StateMachine.TransitionTo(id))
			m_CurrentAnimation = m_Animations[id];
	}

	void AnimationController::RemoveAnimation(uint32_t id)
	{
		XYZ_ASSERT(id < m_Animations.size(), "State does not exist");
		if (m_StateMachine.GetCurrentState().GetID() == id)
		{
			for (auto animation : m_Animations)
			{
				if (animation)
				{
					m_CurrentAnimation = animation;
					break;
				}
			}
		}
		m_Animations[id] = nullptr;
		m_StateMachine.DestroyState(id);
	}

	const Ref<Animation>& AnimationController::GetCurrentAnimation() const
	{
		XYZ_ASSERT(m_CurrentAnimation, "No animation exists");
		return m_CurrentAnimation;
	}
}