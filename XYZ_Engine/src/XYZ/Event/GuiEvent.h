#pragma once
#include "Event.h"


namespace XYZ {

	struct ClickEvent : public Event
	{
		ClickEvent()	
			: m_Type(EventType::Click)
		{
		}

		virtual EventType GetEventType() const override { return m_Type; }

		static EventType GetStaticType()
		{
			return EventType::Click;
		}

	private:
		EventType m_Type;
	};

	struct ReleaseEvent : public Event
	{
		ReleaseEvent()
			: m_Type(EventType::Release)
		{
		}

		virtual EventType GetEventType() const override { return m_Type; }
		static EventType GetStaticType()
		{
			return EventType::Release;
		}

	private:
		EventType m_Type;
	};

	struct HooverEvent : public Event
	{
		HooverEvent()
			: m_Type(EventType::Hoover)
		{
		}
		virtual EventType GetEventType() const override { return m_Type; }
		static EventType GetStaticType()
		{
			return EventType::Hoover;
		}
	private:
		EventType m_Type;
	};

}