#pragma once
#include "Event.h"
#include "EventHandler.h"

namespace XYZ {

	template <typename ...Events>
	class EventSystem
	{
	public:
		template <typename Event>
		bool Execute(Event& event)
		{
			auto& handler = std::get<EventHandler<Event>>(m_Events);
			return handler.ExecuteCallbacks(event);
		}

		virtual ~EventSystem() = default;
		
		template <typename Event>
		size_t RegisterCallback(const EventCallback<Event>& func)
		{
			auto& handler = std::get<EventHandler<Event>>(m_Events);
			return handler.AddCallback(func);
		}

		template <typename Event>
		bool UnRegisterCallback(size_t id)
		{
			auto& handler = std::get<EventHandler<Event>>(m_Events);
			return handler.RemoveCallback(id);
		}

	private:
		std::tuple<EventHandler<Events>...> m_Events;
	};

	
}