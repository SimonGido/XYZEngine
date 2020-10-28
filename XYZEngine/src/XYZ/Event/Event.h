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
		None = 0,

		WindowClosed,
		WindowResized,

		KeyPressed,
		KeyReleased,
		KeyTyped,

		MouseButtonPressed,
		MouseButtonReleased,

		MouseScroll,
		MouseMoved,

		Click,
		Release,
		Checked,
		Hoover,
		UnHoover
	};

#define EVENT_CLASS_TYPE(Type) static EventComponent GetStaticType() { return EventComponent::Type; }
								


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