#include "stdafx.h"
#include "StateMachine.h"


namespace XYZ {

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

	State StateMachine::CreateState(const std::string& name)
	{
		for (auto state : m_StatesMap)
		{
			if (state.Name == name)
			{
				XYZ_LOG_ERR("State with name ", name, " already exists");
				break;
			}
		}
		
		XYZ_ASSERT(m_NextFreeBit < sc_MaxBit, "State machine can has only %d states", sc_MaxBit);
		State state;
		state.m_ID = m_NextFreeBit++;
		m_StatesInitialized |= BIT(state.m_ID);

		m_StatesMap[state.m_ID].Name = name;
		m_StatesMap[state.m_ID].State = state;
		return state;
	}

	bool StateMachine::TransitionTo(uint32_t id)
	{
		auto& current = m_StatesMap[m_CurrentState].State;
		// Check if id of state is in allowed transitions of the current state or check if the current state has allowed any transitions;	
		XYZ_ASSERT(m_StatesInitialized & BIT(id), "State was not registered in this state machine");
		if ((BIT(id) & current.m_AllowedTransitionsTo)
		|| (sc_Any	 & current.m_AllowedTransitionsTo))
		{
			m_CurrentState = id;
			return true;
		}
		return false;
	}


	void StateMachine::RenameState(uint32_t id, const std::string& name)
	{
		XYZ_ASSERT(m_StatesInitialized & BIT(id), "State with id ", id, "is not initialized");
		m_StatesMap[id].Name = name;
	}

	void StateMachine::SetDefaultState(uint32_t id)
	{
		XYZ_ASSERT(m_StatesInitialized & BIT(id), "State with id ", id, "is not initialized");
		XYZ_ASSERT(id < m_NextFreeBit, "State was not registered in this state machine");
	}


}