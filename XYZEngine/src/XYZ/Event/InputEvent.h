#pragma once
#include "Event.h"
#include "XYZ/Core/KeyCodes.h"

namespace XYZ {
	/*! @class KeyPressedEvent
	*	@brief Occurs when a key is pressed
	*/
	class KeyPressedEvent : public Event
	{
	public:
		KeyPressedEvent(int key, int mod)
			:m_Key(key), m_Mod(mod),m_Type(EventType::KeyPressed)
		{
		}
		virtual int GetKey() const { return m_Key; }
		virtual int GetMod() const { return m_Mod; }
		virtual bool IsKeyPressed(KeyCode code) const { return m_Key == ToUnderlying(code); }

		virtual EventType GetEventType() const override { return m_Type; }

		static EventType GetStaticType()
		{
			return EventType::KeyPressed;
		}
	private:
		int m_Key;
		int m_Mod;
		EventType m_Type;
	};

	/*! @class KeyReleasedEvent
	*	@brief Occurs when a key is released
	*/
	class KeyReleasedEvent : public Event
	{
	public:
		KeyReleasedEvent(int key)
			:m_Key(key),m_Type(EventType::KeyReleased)
		{
		}
		virtual int GetKey() const { return m_Key; }
		virtual bool IsKeyReleased(KeyCode code) const { return m_Key == ToUnderlying(code); }
		virtual EventType GetEventType() const override { return m_Type; }
		static EventType GetStaticType()
		{
			return EventType::KeyReleased;
		}
	private:
		int m_Key;
		EventType m_Type;
	};

	/*! @class KeyTypedEvent
	*	@brief Occurs when a key is pressed and then released
	*/
	class KeyTypedEvent : public Event
	{
	public:
		KeyTypedEvent(int key)
			:m_Key(key),m_Type(EventType::KeyTyped)
		{
		}
		virtual int GetKey() const { return m_Key; }
		virtual EventType GetEventType() const override { return m_Type; }

		static EventType GetStaticType()
		{
			return EventType::KeyTyped;
		}
	private:
		int m_Key;
		EventType m_Type;
	};

	/*! @class MouseMovedEvent
	*	@brief Occurs when the mouse pointer moves
	*/
	class MouseMovedEvent : public Event
	{
	public:
		MouseMovedEvent(int x, int y)
			: m_X(x), m_Y(y), m_Type(EventType::MouseMoved)
		{
			
		}
		virtual int GetX() const { return m_X; }
		virtual int GetY() const { return m_Y; }

		virtual EventType GetEventType() const override { return m_Type; }
		static EventType GetStaticType()
		{
			return EventType::MouseMoved;
		}
	private:
		int m_X, m_Y;
		EventType m_Type;
	};

	/*! @class MouseButtonPressEvent
	*	@brief Occurs when a mouse button is pressed
	*/
	class MouseButtonPressEvent : public Event
	{
	public:
		MouseButtonPressEvent(int button)
			:m_Button(button), m_Type(EventType::MouseButtonPressed)
		{
			
		};

		inline int GetButton() const { return m_Button; }
		virtual EventType GetEventType() const override { return m_Type; }
		static EventType GetStaticType()
		{
			return EventType::MouseButtonPressed;
		}
	private:
		int m_Button;
		EventType m_Type;
	};

	/*! @class MouseButtonReleaseEvent
	*	@brief Occurs when a mouse button is released
	*/
	class MouseButtonReleaseEvent : public Event
	{
	public:
		MouseButtonReleaseEvent(int button)
			:m_Button(button),m_Type(EventType::MouseButtonReleased)
		{
		};

		inline int GetButton() const { return m_Button; }
		virtual EventType GetEventType() const override { return m_Type; }
		
		static EventType GetStaticType()
		{
			return EventType::MouseButtonReleased;
		}
	private:
		int m_Button;
		EventType m_Type;
	};

	/*! @class MouseScrollEvent
	*	@brief Occurs when the mousewhell scrolls
	*/
	class MouseScrollEvent :public Event
	{
	public:
		MouseScrollEvent(float xOffset, float yOffset)
			:m_XOffset(xOffset), m_YOffset(yOffset),m_Type(EventType::MouseScroll)
		{
		};

		inline double GetOffsetX() const { return m_XOffset; }
		inline double GetOffsetY() const { return m_YOffset; }

		virtual EventType GetEventType() const override { return m_Type; }

		static EventType GetStaticType()
		{
			return EventType::MouseScroll;
		}
	private:
		float m_XOffset;
		float m_YOffset;
		EventType m_Type;
	};
}