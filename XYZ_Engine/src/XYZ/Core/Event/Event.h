#pragma once 
#include "XYZ/Core/Logger.h"

/*
	Very simple event system, requires an update in the future.
*/


namespace XYZ {

	/*! @class EventType
	*	@brief Types of events that can be triggered in the engine. User-defined events are classified as 'Custom'
	*/
	enum class EventType
	{
		None = 0,

		WindowClosed,
		WindowResized,

		KeyPressed,
		KeyReleased,
		KeyTyped,

		MouseButtonPressed,
		MouseButtonReleased,

		MouseScroll,
		MouseMoved,

		Custom
	};


	/*! @class Event
	*	@brief Describes an event and its handlers
	*/
	class Event
	{
	public:
		virtual EventType GetEventType() const { return m_Type; };
		bool& IsHandled() { return m_Handled; }
		void SetHandled() { m_Handled = true; }

	protected:
		bool m_Handled = false;
		EventType m_Type = EventType::None;
	};


	using event_ptr = std::shared_ptr<Event>;

	/*! @class WindowResizeEvent
	*	@brief Occurs when a window is resized
	*/
	class WindowResizeEvent : public Event
	{
	public:
		WindowResizeEvent(int width, int height)
			:m_Width(width), m_Height(height)
		{
			XYZ_LOG_INFO("Window: Resized window width: ", width, " height: ", height);
			m_Type = EventType::WindowResized;
		}

		virtual std::pair<int, int> GetWinSize() const { return { (int)m_Width, (int)m_Height }; }
		inline int GetWidth() const { return m_Width; }
		inline int GetHeight() const { return m_Height; }

	private:
		int m_Width;
		int m_Height;

	};

	/*! @class WindowCloseEvent
	*	@brief Occurs when a window is closed
	*/
	class WindowCloseEvent : public Event
	{
	public:
		WindowCloseEvent()
		{
			XYZ_LOG_INFO("Window: Window closed");
			m_Type = EventType::WindowClosed;
		}
	};

	/*! @class KeyPressedEvent
	*	@brief Occurs when a key is pressed
	*/
	class KeyPressedEvent : public Event
	{
	public:
		KeyPressedEvent(int key, int mod)
			:m_Key(key), m_Mod(mod)
		{
			m_Type = EventType::KeyPressed;
		}
		virtual int GetKey() const { return m_Key; }
		virtual int GetMod() const { return m_Mod; }
	private:
		int m_Key;
		int m_Mod;
	};

	/*! @class KeyReleasedEvent
	*	@brief Occurs when a key is released
	*/
	class KeyReleasedEvent : public Event
	{
	public:
		KeyReleasedEvent(int key)
			:m_Key(key)
		{
			m_Type = EventType::KeyReleased;
		}
		virtual int GetKey() const { return m_Key; }

	private:
		int m_Key;
	};

	/*! @class KeyTypedEvent
	*	@brief Occurs when a key is pressed and then released
	*/
	class KeyTypedEvent : public Event
	{
	public:
		KeyTypedEvent(int key)
			:m_Key(key)
		{
			m_Type = EventType::KeyTyped;
		}
		virtual int GetKey() const { return m_Key; }

	private:
		int m_Key;
	};

	/*! @class MouseMovedEvent
	*	@brief Occurs when the mouse pointer moves
	*/
	class MouseMovedEvent : public Event
	{
	public:
		MouseMovedEvent(int x, int y)
			: m_X(x), m_Y(y)
		{
			m_Type = EventType::MouseMoved;
		}
		virtual int GetX() const { return m_X; }
		virtual int GetY() const { return m_Y; }

	private:
		int m_X, m_Y;
	};

	/*! @class MouseButtonPressEvent
	*	@brief Occurs when a mouse button is pressed
	*/
	class MouseButtonPressEvent : public Event
	{
	public:
		MouseButtonPressEvent(int button)
			:m_Button(button)
		{
			m_Type = EventType::MouseButtonPressed;
		};

		inline int GetButton() const { return m_Button; }

	private:
		int m_Button;
	};

	/*! @class MouseButtonReleaseEvent
	*	@brief Occurs when a mouse button is released
	*/
	class MouseButtonReleaseEvent : public Event
	{
	public:
		MouseButtonReleaseEvent(int button)
			:m_Button(button)
		{
			m_Type = EventType::MouseButtonReleased;
		};

		inline int GetButton() const { return m_Button; }

	private:
		int m_Button;
	};

	/*! @class MouseScrollEvent
	*	@brief Occurs when the mousewhell scrolls
	*/
	class MouseScrollEvent :public Event
	{
	public:
		MouseScrollEvent(float xOffset, float yOffset)
			:m_XOffset(xOffset), m_YOffset(yOffset)
		{
			m_Type = EventType::MouseScroll;
		};

		inline double GetOffsetX() const { return m_XOffset; }
		inline double GetOffsetY() const { return m_YOffset; }
	private:
		float m_XOffset;
		float m_YOffset;
	};

}