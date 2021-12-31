#pragma once

#include <queue>
#include <bitset>

namespace XYZ {

	
	template <uint32_t NumStates>
	class StateMachine
	{
	public:
		StateMachine(uint32_t defaultId = 0);
	
		bool TransitionTo(uint32_t id);
	
		void SetState(uint32_t id);
		void SetAllowTransition(uint32_t fromID, uint32_t toID, bool value);
	
		inline bool		CanTransitTo(uint32_t id) const { return m_CurrentState->AllowedTransitions.test(id); }
		inline uint32_t GetCurrentState()		  const { return m_CurrentState->ID; }
		
	private:
		struct State
		{
			std::bitset<NumStates> AllowedTransitions;
			uint32_t			   ID;
		};
	
		State* m_CurrentState;
		State  m_States[NumStates];
	};
	
	template<uint32_t NumStates>
	inline StateMachine<NumStates>::StateMachine(uint32_t defaultId)
	{
		m_CurrentState = &m_States[defaultId];
	}
	
	template<uint32_t NumStates>
	inline bool StateMachine<NumStates>::TransitionTo(uint32_t id)
	{
		if (m_CurrentState->AllowedTransitions.test(id))
		{
			m_CurrentState = &m_States[id];
			return true;
		}
		return false;
	}
	
	template<uint32_t NumStates>
	inline void StateMachine<NumStates>::SetState(uint32_t id)
	{
		m_CurrentState = &m_States[id];
	}
	
	template<uint32_t NumStates>
	inline void StateMachine<NumStates>::SetAllowTransition(uint32_t fromID, uint32_t toID, bool value)
	{
		m_States[fromID].AllowedTransitions.set(toID, value);
	}


}
