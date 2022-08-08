#pragma once
#include <tuple>
#include <functional>
#include <vector>

namespace XYZ {

	template <typename Event>
	class EventHandler
	{
	public:	
		size_t AddCallback(const EventCallback<Event>& func)
		{
			m_Callbacks.push_back({ m_NextID, func });
			return m_NextID++;
		}

		bool RemoveCallback(size_t id)
		{
			for (size_t i = 0; i < m_Callbacks.size(); ++i)
			{
				if (m_Callbacks[i].ID == id)
				{
					m_Callbacks.erase(m_Callbacks.begin() + i);
					return true;
				}
			}
			return false;
		}

		bool ExecuteCallbacks(Event& event)
		{
			for (auto& it : m_Callbacks)
			{
				if (it.Callback(event))
					return true;
			}
			return false;
		}


	private:
		struct CallbackPack
		{
			size_t ID;
			EventCallback<Event> Callback;
		};
		std::vector<CallbackPack> m_Callbacks;
		size_t m_NextID = 0;
	};


}
