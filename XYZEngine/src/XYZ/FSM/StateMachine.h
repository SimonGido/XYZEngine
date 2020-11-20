#pragma once

#include <queue>
#include <bitset>

namespace XYZ {

	
	template <uint32_t NumStates>
	class State
	{
	public:
		State() = default;
		State(uint32_t id, const std::bitset<NumStates>& allowedTransitions)
			: m_ID(id), m_AllowedTransitionsTo(allowedTransitions)
		{}

		void SetAllowedTransitions(const std::bitset<NumStates>& allowedTransitionsToBitset)
		{
			m_AllowedTransitionsTo = allowedTransitionsToBitset;
		}
		void AllowTransition(uint32_t stateIndex)
		{
			m_AllowedTransitionsTo |= BIT(stateIndex);
		}
		void DisallowTransition(uint32_t stateIndex)
		{
			m_AllowedTransitionsTo &= ~BIT(stateIndex);
		}
		bool CanTransitTo(uint32_t stateIndex) const
		{
			return m_AllowedTransitionsTo & BIT(stateIndex);
		}

		uint32_t GetID() const { return m_ID; }
		std::bitset<NumStates> GetAllowedTransitions() const { return m_AllowedTransitionsTo; }
	private:
		uint32_t m_ID;
		std::bitset<NumStates> m_AllowedTransitionsTo;
	};

	// TODO: use maybe std::bitset instead of uint32_t , it will result in more states
	template <uint32_t NumStates>
	class StateMachine
	{
	public:
		State<NumStates>& CreateState()
		{
			XYZ_ASSERT(m_NextFreeBit < NumStates, "State machine can has only %d states", NumStates);
			State<NumStates> state(m_NextFreeBit++, 0);
			m_StatesInitialized |= BIT(state.GetID());
			m_States[state.GetID()] = state;
			return m_States[state.GetID()];
		}

		bool TransitionTo(uint32_t id)
		{
			auto& current = m_States[m_CurrentState];
			// Check if id of state is in allowed transitions of the current state or check if the current state has allowed any transitions;	
			XYZ_ASSERT(m_StatesInitialized & BIT(id), "State was not registered in this state machine");
			if (current.GetAllowedTransitions().test(id))
			{
				m_CurrentState = id;
				return true;
			}
			return false;
		}

		void SetDefaultState(uint32_t id)
		{
			XYZ_ASSERT(m_StatesInitialized & BIT(id), "State with id ", id, "is not initialized");
			XYZ_ASSERT(id < m_NextFreeBit, "State was not registered in this state machine");
			m_CurrentState = id;
		}

		inline const State<NumStates>& GetCurrentState() const 
		{ 
			return m_States[m_CurrentState]; 
		}
		inline State<NumStates>& GetState(uint32_t id) 
		{ 
			XYZ_ASSERT(m_StatesInitialized & BIT(id), "State with id ", id, "is not initialized");
			return m_States[id]; 
		}

		inline uint32_t GetNumStates() const { return m_NextFreeBit; }

		bool IsStateInitialized(uint32_t id) const 
		{ 
			return m_StatesInitialized & BIT(id); 
		}
	private:
		uint32_t m_CurrentState;

		uint32_t m_NextFreeBit = 0;
		uint32_t m_StatesInitialized = 0;

		State<NumStates> m_States[NumStates];
	};


}