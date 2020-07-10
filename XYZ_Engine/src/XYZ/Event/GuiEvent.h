#pragma once
#include "Event.h"


namespace XYZ {

	struct ClickEvent : public Event
	{
		ClickEvent()	
			: m_Component(EventComponent::Click)
		{
		}

		virtual EventComponent GetEventComponent() const override { return m_Component; }

		static EventComponent GetStaticComponent()
		{
			return EventComponent::Click;
		}

	private:
		EventComponent m_Component;
	};

	struct CheckedEvent : public Event
	{
		CheckedEvent()
			: m_Component(EventComponent::Checked)
		{
		}

		virtual EventComponent GetEventComponent() const override { return m_Component; }

		static EventComponent GetStaticComponent()
		{
			return EventComponent::Checked;
		}

	private:
		EventComponent m_Component;
	};

	struct ReleaseEvent : public Event
	{
		ReleaseEvent()
			: m_Component(EventComponent::Release)
		{
		}

		virtual EventComponent GetEventComponent() const override { return m_Component; }
		static EventComponent GetStaticComponent()
		{
			return EventComponent::Release;
		}

	private:
		EventComponent m_Component;
	};


	struct HooverEvent : public Event
	{
		HooverEvent()
			: m_Component(EventComponent::Hoover)
		{
		}
		virtual EventComponent GetEventComponent() const override { return m_Component; }
		static EventComponent GetStaticComponent()
		{
			return EventComponent::Hoover;
		}
	private:
		EventComponent m_Component;
	};

	struct UnHooverEvent : public Event
	{
		UnHooverEvent()
			: m_Component(EventComponent::UnHoover)
		{
		}
		virtual EventComponent GetEventComponent() const override { return m_Component; }
		static EventComponent GetStaticComponent()
		{
			return EventComponent::UnHoover;
		}
	private:
		EventComponent m_Component;
	};
}