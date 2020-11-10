#pragma once

#include <queue>

namespace XYZ {

	
	class State
	{
	public:
		void SetAllowedTransitions(uint32_t allowedTransitionsToBitset);
		void AllowTransition(uint32_t stateIndex);
		void DisallowTransition(uint32_t stateIndex);
		bool CanTransitTo(uint32_t stateIndex);

		uint32_t GetID() const { return m_ID; }
		uint32_t GetAllowedTransitions() const { return m_AllowedTransitionsTo; }
	private:
		uint32_t m_ID = 0;
		uint32_t m_AllowedTransitionsTo = 0;

		friend class StateMachine;
	};

	// TODO: use maybe std::bitset instead of uint32_t , it will result in more states
	class StateMachine
	{
	public:
		State& CreateState();

		bool TransitionTo(uint32_t id);

		void SetDefaultState(uint32_t id);

		inline const State& GetCurrentState() const 
		{ 
			return m_States[m_CurrentState]; 
		}
		inline State& GetState(uint32_t id) 
		{ 
			XYZ_ASSERT(m_StatesInitialized & BIT(id), "State with id ", id, "is not initialized");
			return m_States[id]; 
		}

		inline uint32_t GetNumStates() const { return m_NextFreeBit; }

		bool IsStateInitialized(uint32_t id) const { return m_StatesInitialized & BIT(id); }

		static uint32_t GetAny() { return sc_Any; }
		static constexpr uint32_t GetMaxBit() { return sc_MaxBit; }
	private:
		static constexpr uint32_t sc_MaxBit = 31;
		static constexpr uint32_t sc_Any = (1 << 31);

		uint32_t m_CurrentState;

		uint32_t m_NextFreeBit = 0;
		uint32_t m_StatesInitialized = 0;

		State m_States[sc_MaxBit];
	};


}