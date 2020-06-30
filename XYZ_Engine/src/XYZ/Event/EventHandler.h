#pragma once
#include <tuple>
#include <functional>
#include <vector>

namespace XYZ {

	template <typename Event>
	using EventCallback = std::function<void(Event)>;

	template <typename Event>
	class EventHandler
	{
	public:
		void AddCallback(const EventCallback<Event>& func)
		{
			m_Callbacks.push_back(func);
		}

		void RemoveCallback(const EventCallback<Event>& func)
		{
			auto it = std::find(m_Callbacks.begin(), m_Callbacks.end(), func);
			if (it != m_Callbacks.end())
				m_Callbacks.erase(it);
		}

		void ExecuteCallbacks(Event& event)
		{
			for (auto& callback : m_Callbacks)
				callback(event);
		}

	private:
		std::vector<EventCallback<Event>> m_Callbacks;
	};


}
