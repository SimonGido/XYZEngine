#pragma once
#include "Event.h"
#include "EventHandler.h"

namespace XYZ {

	template <typename ...Event>
	class EventSystem
	{
	public:
		template <typename Event>
		void Execute(Event& event)
		{
			auto& handler = std::get<EventHandler<Event>>(m_Events);
			handler.ExecuteCallbacks(event);
		}

		template <typename Event>
		void RegisterCallback(const EventCallback<Event>& func)
		{
			auto& handler = std::get<EventHandler<Event>>(m_Events);
			handler.AddCallback(func);
		}

		template <typename Event>
		void UnRegisterCallback(const EventCallback<Event>& func)
		{
			auto& handler = std::get<EventHandler<Event>>(m_Events);
			handler.RemoveCallback(func);
		}

	private:
		std::tuple<EventHandler<Event>...> m_Events;

	};
}