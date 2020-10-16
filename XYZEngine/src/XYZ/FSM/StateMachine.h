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

		const State& GetCurrentState() const { return m_CurrentState; }

		static uint32_t GetAny() { return sc_Any; }

		State& GetState(uint32_t id) { return m_StatesMap[id].State; }
		const std::string& GetStateName(uint32_t id) { return m_StatesMap[id].Name; }

		const std::unordered_map<uint32_t, StatePair>& GetStatesMap() const { return m_StatesMap; }
		std::unordered_map<uint32_t, StatePair>& GetStatesMap() { return m_StatesMap; }
	private:
		State m_CurrentState;

		uint32_t m_NextFreeBit = 0;

		std::unordered_map<uint32_t, StatePair> m_StatesMap;

		static constexpr uint32_t sc_MaxBit = 31;
		static constexpr uint32_t sc_Any = (1 << 31);
	};


}