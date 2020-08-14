#pragma once

#include <queue>

namespace XYZ {

	

	template <typename T>
	struct State
	{
		T Value;
		uint8_t ID;
		uint32_t Transition;
	};
;

	template <typename T>
	class Machine
	{
	
	public:
		Machine(const std::string& defaultState, const T& value)
		{
			m_CurrentState = &AddState(defaultState, value);
		}

		State<T>& AddState(const std::string& name, const T& value)
		{
			XYZ_ASSERT(m_NameIdMap.find(name) == m_NameIdMap.end(), "State already exists");
			uint8_t id = 0;
			if (!m_FreeIDs.empty())
			{
				id = m_FreeIDs.back();
				m_FreeIDs.pop();
			}
			else
				id = m_NextID++;
			
			m_NameIdMap[name] = id;
			m_States[id] = { value, id, 0 };
			return m_States[id];
		}

		void RemoveState(const std::string& name)
		{
			auto it = m_NameIdMap.find(name);
			XYZ_ASSERT(it != m_NameIdMap.end(), "State does not exist");

			auto state = m_States.find(it->second);
			XYZ_ASSERT(it != m_States.end(), "State does not exist");

			m_FreeIDs.push(it->second.ID);
			m_NameIdMap.erase(it);
			m_States.erase(state);
		}

		void SetStateTransition(const std::string& state, const std::string& transitionToState)
		{
			auto it = m_NameIdMap.find(state);
			XYZ_ASSERT(it != m_NameIdMap.end(), "State does not exist");

			auto transition = m_NameIdMap.find(transitionToState);
			XYZ_ASSERT(transition != m_NameIdMap.end(), "Transition to state does not exist");

			auto stateIt = m_States.find(it->second);
			XYZ_ASSERT(stateIt != m_States.end(), "State does not exist");

			auto transitionIt = m_States.find(transition->second);
			XYZ_ASSERT(transitionIt != m_States.end(), "Transition to state does not exist");


			stateIt->second.Transition |= BIT(transitionIt->second.ID);
		}
		
		State<T>& TransitionTo(const std::string& name)
		{
			auto it = m_NameIdMap.find(name);
			XYZ_ASSERT(it != m_NameIdMap.end(), "Name id does not exist");

			if (m_CurrentState->Transition & BIT(it->second))
			{
				auto state = m_States.find(it->second);
				XYZ_ASSERT(state != m_States.end(), "State does not exist");
				m_CurrentState = &state->second;
			}

			return *m_CurrentState;
		}

		State<T>& TransitionTo(uint8_t id)
		{
			auto state = m_States.find(id);
			XYZ_ASSERT(state != m_States.end(), "State does not exist");

			if (m_CurrentState->Transition & BIT(id))
			{
				auto state = m_States.find(id);
				XYZ_ASSERT(state != m_States.end(), "State does not exist");
				m_CurrentState = &state->second;
			}
		}

		State<T>& GetCurrentState() { return *m_CurrentState; }

		uint8_t GetStateID(const std::string& state) const { return m_States[state].ID; }

	private:
		uint8_t m_NextID = 0;
		State<T>* m_CurrentState = nullptr;

		std::queue<uint8_t> m_FreeIDs;

		std::unordered_map<uint8_t, State<T>> m_States;

		std::unordered_map<std::string, uint8_t> m_NameIdMap;
	};
	
}