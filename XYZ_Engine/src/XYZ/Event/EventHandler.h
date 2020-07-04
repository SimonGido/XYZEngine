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
			for (size_t i = 0; i < m_Callbacks.size(); ++i)
			{
				if (getAddress(m_Callbacks[i]) == getAddress(func))
				{
					m_Callbacks.erase(m_Callbacks.begin() + i);
					break;
				}
			}
		}

		void ExecuteCallbacks(Event& event)
		{
			for (auto& callback : m_Callbacks)
				callback(event);
		}

	private:
		// Little hack for comparison 
		template<typename T, typename... U>
		size_t getAddress(std::function<T(U...)> f) 
		{
			typedef T(fnType)(U...);
			fnType** fnPointer = f.template target<fnType*>();
			return (size_t)*fnPointer;
		}


	private:
		std::vector<EventCallback<Event>> m_Callbacks;
	};


}
