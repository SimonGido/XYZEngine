#pragma once
#include <tuple>
#include <functional>
#include <vector>

namespace XYZ {

	//template <typename Event>
	//using EventCallback = std::function<bool(Event)>;

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

		bool ExecuteCallbacks(Event& event)
		{
			for (auto& callback : m_Callbacks)
			{
				if (callback(event))
					return true;
			}
			return false;
		}

	private:
		// Little hack for comparison 
		template<typename T, typename... U>
		size_t getAddress(std::function<T(U...)> f) 
		{
			Componentdef T(fnComponent)(U...);
			fnComponent** fnPointer = f.template target<fnComponent*>();
			return (size_t)*fnPointer;
		}


	private:
		std::vector<EventCallback<Event>> m_Callbacks;
	};


}
