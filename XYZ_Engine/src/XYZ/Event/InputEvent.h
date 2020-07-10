#pragma once
#include "Event.h"

namespace XYZ {
	/*! @class KeyPressedEvent
	*	@brief Occurs when a key is pressed
	*/
	class KeyPressedEvent : public Event
	{
	public:
		KeyPressedEvent(int key, int mod)
			:m_Key(key), m_Mod(mod),m_Component(EventComponent::KeyPressed)
		{
		}
		virtual int GetKey() const { return m_Key; }
		virtual int GetMod() const { return m_Mod; }

		virtual EventComponent GetEventComponent() const override { return m_Component; }

		static EventComponent GetStaticComponent()
		{
			return EventComponent::KeyPressed;
		}
	private:
		int m_Key;
		int m_Mod;
		EventComponent m_Component;
	};

	/*! @class KeyReleasedEvent
	*	@brief Occurs when a key is released
	*/
	class KeyReleasedEvent : public Event
	{
	public:
		KeyReleasedEvent(int key)
			:m_Key(key),m_Component(EventComponent::KeyReleased)
		{
		
		}
		virtual int GetKey() const { return m_Key; }

		virtual EventComponent GetEventComponent() const override { return m_Component; }
		static EventComponent GetStaticComponent()
		{
			return EventComponent::KeyReleased;
		}
	private:
		int m_Key;
		EventComponent m_Component;
	};

	/*! @class KeyComponentdEvent
	*	@brief Occurs when a key is pressed and then released
	*/
	class KeyComponentdEvent : public Event
	{
	public:
		KeyComponentdEvent(int key)
			:m_Key(key),m_Component(EventComponent::KeyComponentd)
		{
		}
		virtual int GetKey() const { return m_Key; }
		virtual EventComponent GetEventComponent() const override { return m_Component; }

		static EventComponent GetStaticComponent()
		{
			return EventComponent::KeyComponentd;
		}
	private:
		int m_Key;
		EventComponent m_Component;
	};

	/*! @class MouseMovedEvent
	*	@brief Occurs when the mouse pointer moves
	*/
	class MouseMovedEvent : public Event
	{
	public:
		MouseMovedEvent(int x, int y)
			: m_X(x), m_Y(y), m_Component(EventComponent::MouseMoved)
		{
			
		}
		virtual int GetX() const { return m_X; }
		virtual int GetY() const { return m_Y; }

		virtual EventComponent GetEventComponent() const override { return m_Component; }
		static EventComponent GetStaticComponent()
		{
			return EventComponent::MouseMoved;
		}
	private:
		int m_X, m_Y;
		EventComponent m_Component;
	};

	/*! @class MouseButtonPressEvent
	*	@brief Occurs when a mouse button is pressed
	*/
	class MouseButtonPressEvent : public Event
	{
	public:
		MouseButtonPressEvent(int button)
			:m_Button(button), m_Component(EventComponent::MouseButtonPressed)
		{
			
		};

		inline int GetButton() const { return m_Button; }
		virtual EventComponent GetEventComponent() const override { return m_Component; }
		static EventComponent GetStaticComponent()
		{
			return EventComponent::MouseButtonPressed;
		}
	private:
		int m_Button;
		EventComponent m_Component;
	};

	/*! @class MouseButtonReleaseEvent
	*	@brief Occurs when a mouse button is released
	*/
	class MouseButtonReleaseEvent : public Event
	{
	public:
		MouseButtonReleaseEvent(int button)
			:m_Button(button),m_Component(EventComponent::MouseButtonReleased)
		{
		};

		inline int GetButton() const { return m_Button; }
		virtual EventComponent GetEventComponent() const override { return m_Component; }
		
		static EventComponent GetStaticComponent()
		{
			return EventComponent::MouseButtonReleased;
		}
	private:
		int m_Button;
		EventComponent m_Component;
	};

	/*! @class MouseScrollEvent
	*	@brief Occurs when the mousewhell scrolls
	*/
	class MouseScrollEvent :public Event
	{
	public:
		MouseScrollEvent(float xOffset, float yOffset)
			:m_XOffset(xOffset), m_YOffset(yOffset),m_Component(EventComponent::MouseScroll)
		{
		};

		inline double GetOffsetX() const { return m_XOffset; }
		inline double GetOffsetY() const { return m_YOffset; }

		virtual EventComponent GetEventComponent() const override { return m_Component; }

		static EventComponent GetStaticComponent()
		{
			return EventComponent::MouseScroll;
		}
	private:
		float m_XOffset;
		float m_YOffset;
		EventComponent m_Component;
	};
}