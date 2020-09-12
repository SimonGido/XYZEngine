#include "stdafx.h"
#include "StateMachine.h"


namespace XYZ {

	void State::SetAllowedTransitions(uint32_t allowedTransitionsTo)
	{
		m_AllowedTransitionsTo = allowedTransitionsTo;
	}


	State StateMachine::CreateState(const std::string& name)
	{
		XYZ_ASSERT(m_StatesMap.find(name) == m_StatesMap.end(), "Creating state with same name");
		XYZ_ASSERT(m_NextFreeBit < sc_MaxBit, "State machine can has only %d states", sc_MaxBit);
		State state;
		state.m_ID = m_NextFreeBit++;
		m_StatesMap[name] = state;
		
		return state;
	}

	bool StateMachine::TransitionTo(const State& state)
	{
		// Check if id of state is in allowed transitions of the current state or check if the current state has allowed any transitions;
		XYZ_ASSERT(state.m_ID < m_NextFreeBit, "State was not registered in this state machine");
		if ((state.m_ID & m_CurrentState.m_AllowedTransitionsTo) || (m_CurrentState.m_AllowedTransitionsTo & sc_Any))
		{
			m_CurrentState = state;
			return true;
		}

		return false;
	}

	void StateMachine::SetDefaultState(const State& state)
	{
		XYZ_ASSERT(state.m_ID < m_NextFreeBit, "State was not registered in this state machine");
	}


}