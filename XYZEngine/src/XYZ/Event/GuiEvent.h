#pragma once
#include "Event.h"
#include "XYZ/ECS/ECSManager.h"
#include "XYZ/ECS/Entity.h"

namespace XYZ {

	struct ClickEvent : public Event
	{
		ClickEvent(Entity entity)	
			: m_Type(EventType::Click),m_Entity(entity)
		{
		}

		virtual EventType GetEventType() const override { return m_Type; }

		Entity GetEntity() const { return m_Entity; }

		static EventType GetStaticType()
		{
			return EventType::Click;
		}

	private:
		EventType m_Type;
		Entity m_Entity;
	};



	struct ReleaseEvent : public Event
	{
		ReleaseEvent(Entity entity)
			: m_Type(EventType::Release), m_Entity(entity)
		{
		}

		virtual EventType GetEventType() const override { return m_Type; }
		Entity GetEntity() const { return m_Entity; }
		static EventType GetStaticType()
		{
			return EventType::Release;
		}

	private:
		EventType m_Type;
		Entity m_Entity;
	};

	struct DraggedEvent : public Event
	{
		DraggedEvent(Entity entity, float value)
			: m_Type(EventType::Drag), m_Entity(entity), m_Value(value)
		{
		}

		virtual EventType GetEventType() const override { return m_Type; }

		Entity GetEntity() const { return m_Entity; }

		static EventType GetStaticType()
		{
			return EventType::Drag;
		}

	private:
		EventType m_Type;
		Entity m_Entity;
		float m_Value;
	};

	struct CheckedEvent : public Event
	{
		CheckedEvent()
			: m_Type(EventType::Checked)
		{
		}

		virtual EventType GetEventType() const override { return m_Type; }

		static EventType GetStaticType()
		{
			return EventType::Checked;
		}

	private:
		EventType m_Type;
	};
	struct UnCheckedEvent : public Event
	{
		UnCheckedEvent()
			: m_Type(EventType::UnChecked)
		{
		}

		virtual EventType GetEventType() const override { return m_Type; }

		static EventType GetStaticType()
		{
			return EventType::UnChecked;
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

	struct UnHooverEvent : public Event
	{
		UnHooverEvent()
			: m_Type(EventType::UnHoover)
		{
		}
		virtual EventType GetEventType() const override { return m_Type; }
		static EventType GetStaticType()
		{
			return EventType::UnHoover;
		}
	private:
		EventType m_Type;
	};

	struct RectTransformResizedEvent : public Event
	{
		RectTransformResizedEvent(Entity entity)
			: m_Type(EventType::RectTransformResized),m_Entity(entity)
		{}

		Entity GetEntity() const { return m_Entity; }

		virtual EventType GetEventType() const override { return m_Type; }

		static EventType GetStaticType()
		{
			return EventType::RectTransformResized;
		}
	private:
		EventType m_Type;
		Entity m_Entity;
	};
}