#pragma once
#include <XYZ.h>

#include "../Inspectable/Inspectable.h"

namespace XYZ {
	class DeselectedEvent : public Event
	{
	public:
		DeselectedEvent()
			:
			m_Type(EventType::Deselected)
		{}

		virtual EventType GetEventType() const override { return m_Type; }

		static EventType GetStaticType()
		{
			return EventType::Deselected;
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


	class AnimatorSelectedEvent : public Event
	{
	public:
		AnimatorSelectedEvent(const Ref<AnimationController>& controller)
			:
			m_Type(EventType::AnimatorSelected),
			m_Controller(controller)
		{}
		virtual EventType GetEventType() const override { return m_Type; }

		Ref<AnimationController> GetController() const { return m_Controller; }

		static EventType GetStaticType()
		{
			return EventType::AnimatorSelected;
		}

	private:
		EventType m_Type;
		Ref<AnimationController> m_Controller;
	};

	class SpriteSelectedEvent : public Event
	{
	public:
		SpriteSelectedEvent(const Ref<SubTexture2D>& sprite)
			:
			m_Type(EventType::SpriteSelected),
			m_Sprite(sprite)
		{}

		virtual EventType GetEventType() const override { return m_Type; }

		Ref<SubTexture2D> GetSprite() const { return m_Sprite; }

		static EventType GetStaticType()
		{
			return EventType::SpriteSelected;
		}
	private:
		EventType m_Type;
		Ref<SubTexture2D> m_Sprite;
	};
}