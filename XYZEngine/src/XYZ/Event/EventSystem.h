#pragma once
#include "Event.h"
#include "EventHandler.h"

namespace XYZ {

	template <typename ...Events>
	class EventSystem
	{
	public:
		template <typename Event>
		void Execute(Event& event)
		{
			auto& handler = std::get<EventHandler<Event>>(m_Events);
			handler.ExecuteCallbacks(event);
		}

		virtual ~EventSystem() = default;
		
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
		std::tuple<EventHandler<Events>...> m_Events;

	};
}