#pragma once
#include "Animation.h"
#include "XYZ/FSM/StateMachine.h"
#include "XYZ/Scene/Serializable.h"
#include "XYZ/Core/Timestep.h"

namespace XYZ {

	class AnimationController : public RefCount,
								public Serializable
	{
	public:
		void AddAnimation(const std::string& name, const Ref<Animation>& animation);
		void RemoveAnimation(const std::string& name);
		void SetDefaultAnimation(const std::string& name);
		void Update(Timestep ts);

		StateMachine& GetStateMachine() { return m_StateMachine; }
		const Ref<Animation>& GetCurrentAnimation() const;
	private:
		Ref<Animation> m_CurrentAnimation;
		std::unordered_map<uint32_t, Ref<Animation>> m_Animations;
		std::unordered_map<std::string, uint32_t> m_StatesMap;

		StateMachine m_StateMachine;
	};



}