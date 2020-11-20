#pragma once
#include "Event.h"
#include "XYZ/ECS/ECSManager.h"


namespace XYZ {

	struct ClickEvent : public Event
	{
		ClickEvent(uint32_t entity)	
			: m_Type(EventType::Click),m_Entity(entity)
		{
		}

		virtual EventType GetEventType() const override { return m_Type; }

		uint32_t GetEntity() const { return m_Entity; }

		static EventType GetStaticType()
		{
			return EventType::Click;
		}

	private:
		EventType m_Type;
		uint32_t m_Entity;
	};



	struct ReleaseEvent : public Event
	{
		ReleaseEvent(uint32_t entity)
			: m_Type(EventType::Release), m_Entity(entity)
		{
		}

		virtual EventType GetEventType() const override { return m_Type; }
		uint32_t GetEntity() const { return m_Entity; }
		static EventType GetStaticType()
		{
			return EventType::Release;
		}

	private:
		EventType m_Type;
		uint32_t m_Entity;
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

	struct CanvasRenderer;
	struct RectTransform;
	struct CanvasRendererRebuildSpecification
	{
		virtual void Rebuild(uint32_t entity, ECS::ECSManager& ecs) = 0;
	};

	
	struct CanvasRendererRebuildEvent : public Event
	{
		CanvasRendererRebuildEvent(uint32_t entity, CanvasRendererRebuildSpecification& specification)
			: 
			m_Type(EventType::CanvasRendererRebuild),
			m_Entity(entity),
			m_Specification(specification)
		{}

		virtual EventType GetEventType() const override { return m_Type; }

		uint32_t GetEntity() const { return m_Entity; }
		CanvasRendererRebuildSpecification& GetSpecification() { return m_Specification; }

		static EventType GetStaticType()
		{
			return EventType::CanvasRendererRebuild;
		}
	private:
		EventType m_Type;
		uint32_t m_Entity;
		CanvasRendererRebuildSpecification& m_Specification;
	};
}