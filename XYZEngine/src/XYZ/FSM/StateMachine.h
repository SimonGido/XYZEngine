#pragma once

#include <queue>

namespace XYZ {

	
	class State
	{
	public:
		void SetAllowedTransitions(uint32_t allowedTransitionsToBitset);
		void AllowTransition(uint32_t stateIndex);

		bool CanTransitTo(uint32_t stateIndex);

		uint32_t GetAllowedTransitions() const { return m_AllowedTransitionsTo; }

		uint32_t GetID() const { return m_ID; }

	private:
		uint32_t m_ID = 0;
		uint32_t m_AllowedTransitionsTo = 0;

		friend class StateMachine;
	};

	// TODO: use maybe std::bitset instead of uint32_t , it will result in more states
	class StateMachine
	{
	public:
		struct StatePair
		{
			State State;
			std::string Name;
		};
	public:
		State CreateState(const std::string& name);

		bool TransitionTo(const State& state);

		void SetDefaultState(const State& state);

		void RenameState(uint32_t id, const std::string& name);

		inline const State& GetCurrentState() const { return m_CurrentState; }

		inline State& GetState(uint32_t id) 
		{ 
			XYZ_ASSERT(m_StatesInitialized & BIT(id), "State with id ", id, "is not initialized");
			return m_StatesMap[id].State; 
		}

		inline const std::string& GetStateName(uint32_t id) 
		{
			XYZ_ASSERT(m_StatesInitialized & BIT(id), "State with id ", id, "is not initialized");
			return m_StatesMap[id].Name; 
		}
		inline uint32_t GetNumStates() const { return m_NextFreeBit; }

		bool IsStateInitialized(uint32_t id) const { return m_StatesInitialized & BIT(id); }

		static uint32_t GetAny() { return sc_Any; }
		static constexpr uint32_t GetMaxBit() { return sc_MaxBit; }
	private:
		static constexpr uint32_t sc_MaxBit = 31;
		static constexpr uint32_t sc_Any = (1 << 31);

		State m_CurrentState;

		uint32_t m_NextFreeBit = 0;
		uint32_t m_StatesInitialized = 0;

		StatePair m_StatesMap[sc_MaxBit];
	};


}