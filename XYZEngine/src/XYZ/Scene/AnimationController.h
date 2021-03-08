#pragma once
#include "Animation.h"
#include "XYZ/FSM/StateMachine.h"
#include "XYZ/Asset/Asset.h"
#include "XYZ/Core/Timestep.h"


namespace XYZ {

	class AnimationController : public Asset
	{
	public:
		void Update(Timestep ts);
		void TransitionTo(uint32_t id);
		void RemoveAnimation(uint32_t id);
		uint32_t AddAnimation(Ref<Animation> animation);

		StateMachine<32>& GetStateMachine() { return m_StateMachine; }
		const Ref<Animation>& GetCurrentAnimation() const;
	
	private:
		Ref<Animation> m_CurrentAnimation = nullptr;
		std::vector<Ref<Animation>> m_Animations;

		StateMachine<32> m_StateMachine;
	};
}