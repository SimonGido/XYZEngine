#pragma once

#include <queue>

namespace XYZ {

	class State
	{
	public:
		void SetAllowedTransitions(uint32_t allowedTransitionsTo);

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
		State CreateState(const std::string& name);

		bool TransitionTo(const State& state);

		void SetDefaultState(const State& state);

		const State& GetCurrentState() const { return m_CurrentState; }

		static uint32_t GetAny() { return sc_Any; }
	private:
		State m_CurrentState;

		uint32_t m_NextFreeBit = 0;

		std::unordered_map<std::string, State> m_StatesMap;

		static constexpr uint32_t sc_MaxBit = 31;
		static constexpr uint32_t sc_Any = (1 << 31);
	};


}