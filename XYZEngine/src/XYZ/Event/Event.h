#pragma once 
#include "XYZ/Core/Logger.h"

/*
	Very simple event system, requires an update in the future.
*/


namespace XYZ {

	/*! @class EventType
	*	@brief Components of events that can be triggered in the engine. User-defined events are classified as 'Custom'
	*/
	enum class EventType
	{		
		#include "CoreEventTypes.def"
		#include "../../XYZEditor/src/Event/EditorEventTypes.def"
	};


	template <typename Type>
	using EventCallback = std::function<void(Type)>;

	/*! @class Event
	*	@brief Describes an event and its handlers
	*/
	class Event
	{
	public:
		virtual EventType GetEventType() const = 0;

		bool Handled = false;
	};

	class EventCaller
	{
	public:
		void RegisterCallback(const std::function<void(Event&)>& callback)
		{
			m_Callback = callback;
		}
		void Execute(Event& event)
		{
			XYZ_ASSERT(m_Callback, "Event caller callback not initialized");
			m_Callback(event);
		}
	private:
		std::function<void(Event&)> m_Callback;
	};
	
	class EventDispatcher
	{
	public:
		EventDispatcher(Event& event)
			: m_Event(event)
		{
		}

		template <typename T, typename F>
		bool Dispatch(const F& func)
		{
			if (m_Event.GetEventType() == T::GetStaticType() && !m_Event.Handled)
			{
				m_Event.Handled = func(static_cast<T&>(m_Event));
				return m_Event.Handled;
			}
			return false;
		}
	private:
		Event& m_Event;
	};
}