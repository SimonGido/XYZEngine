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
		virtual void Rebuild(CanvasRenderer* renderer, RectTransform* transform) = 0;
	};

	
	struct CanvasRendererRebuildEvent : public Event
	{
		CanvasRendererRebuildEvent(CanvasRenderer* renderer, RectTransform* transform, CanvasRendererRebuildSpecification& specification)
			: 
			m_Type(EventType::CanvasRendererRebuild),
			m_Renderer(renderer),
			m_Transform(transform),
			m_Specification(specification)
		{}

		virtual EventType GetEventType() const override { return m_Type; }

		CanvasRenderer* GetRenderer() { return m_Renderer; }
		RectTransform* GetTransform() { return m_Transform; }
		CanvasRendererRebuildSpecification& GetSpecification() { return m_Specification; }

		static EventType GetStaticType()
		{
			return EventType::CanvasRendererRebuild;
		}
	private:
		EventType m_Type;
		CanvasRenderer* m_Renderer;
		RectTransform* m_Transform;
		CanvasRendererRebuildSpecification& m_Specification;
	};
}