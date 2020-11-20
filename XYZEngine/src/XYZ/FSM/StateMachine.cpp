#include "stdafx.h"
#include "StateMachine.h"


namespace XYZ {
	State::State(uint32_t id, uint32_t allowedTransitions)
		:
		m_ID(id),m_AllowedTransitionsTo(allowedTransitions)
	{
	}
	void State::SetAllowedTransitions(uint32_t allowedTransitionsToBitset)
	{
		m_AllowedTransitionsTo = allowedTransitionsToBitset;
	}

	void State::AllowTransition(uint32_t stateIndex)
	{
		m_AllowedTransitionsTo |= BIT(stateIndex);
	}
	void State::DisallowTransition(uint32_t stateIndex)
	{
		m_AllowedTransitionsTo &= ~BIT(stateIndex);
	}
	bool State::CanTransitTo(uint32_t stateIndex)
	{
		return m_AllowedTransitionsTo & BIT(stateIndex);
	}
}