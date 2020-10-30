#pragma once
#include <XYZ.h>

#include "../Inspectable/Inspectable.h"

namespace XYZ {

	// Inspector panel is last to receive event and rest of panels overwrite event which will hold inspectable
	class InspectableSelectedEvent : public Event
	{
	public:
		InspectableSelectedEvent(Inspectable* inspectable)
			:
			m_Type(EventType::InspectableSelected),
			m_Inspectable(inspectable)
		{}
	
		virtual EventType GetEventType() const override { return m_Type; }
		inline Inspectable* GetInspectable() { return m_Inspectable; }

		static EventType GetStaticType()
		{
			return EventType::InspectableSelected;
		}
	private:
		EventType m_Type;
		Inspectable* m_Inspectable;
	};


	class InspectableDeselectedEvent : public Event
	{
	public:
		InspectableDeselectedEvent()
			: m_Type(EventType::InspectableDeselected)
		{}

		virtual EventType GetEventType() const override { return m_Type; }

		static EventType GetStaticType()
		{
			return EventType::InspectableDeselected;
		}
	private:
		EventType m_Type;
	};



	class EntitySelectedEvent : public Event
	{
	public:
		EntitySelectedEvent(Entity entity)
			:
			m_Type(EventType::EntitySelected),
			m_Entity(entity)
		{}

		virtual EventType GetEventType() const override { return m_Type; }
		Entity GetEntity() const { return m_Entity; }

		static EventType GetStaticType()
		{
			return EventType::EntitySelected;
		}

	private:
		EventType m_Type;
		Entity m_Entity;
	};


	class EntityDeselectedEvent : public Event
	{
	public:
		EntityDeselectedEvent()
			:
			m_Type(EventType::EntityDeselected)
		{}

		virtual EventType GetEventType() const override { return m_Type; }

		static EventType GetStaticType()
		{
			return EventType::EntityDeselected;
		}

	private:
		EventType m_Type;
		Entity m_Entity;
	};
}