#pragma once 
#include "XYZ/Core/Logger.h"

/*
	Very simple event system, requires an update in the future.
*/


namespace XYZ {

	/*! @class EventComponent
	*	@brief Components of events that can be triggered in the engine. User-defined events are classified as 'Custom'
	*/
	enum class EventComponent
	{
		None = 0,

		WindowClosed,
		WindowResized,

		KeyPressed,
		KeyReleased,
		KeyComponentd,

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

#define EVENT_CLASS_Component(Component) static EventComponent GetStaticComponent() { return EventComponent::Component; }
								


	template <typename Component>
	using EventCallback = std::function<void(Component)>;

	/*! @class Event
	*	@brief Describes an event and its handlers
	*/
	class Event
	{
	public:
		virtual EventComponent GetEventComponent() const = 0;

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
			if (m_Event.GetEventComponent() == T::GetStaticComponent())
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