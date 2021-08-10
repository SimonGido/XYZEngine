#pragma once
#include "Event.h"


namespace XYZ {
	/*! @class WindowResizeEvent
	*	@brief Occurs when a window is resized
	*/
	class WindowResizeEvent : public Event
	{
	public:
		WindowResizeEvent(int width, int height)
			:m_Width(width), m_Height(height),m_Type(EventType::WindowResized)
		{
			XYZ_CORE_INFO("Window: Resized window width: {0}  height: {1}", width, height);
		}


		virtual std::pair<int, int> GetWinSize() const { return { (int)m_Width, (int)m_Height }; }
		inline int GetWidth() const { return m_Width; }
		inline int GetHeight() const { return m_Height; }

		virtual EventType GetEventType() const override { return m_Type; }
		static EventType GetStaticType()
		{
			return EventType::WindowResized;
		}

	private:
		int m_Width;
		int m_Height;
		EventType m_Type;
	};

	/*! @class WindowCloseEvent
	*	@brief Occurs when a window is closed
	*/
	class WindowCloseEvent : public Event
	{
	public:
		WindowCloseEvent()
			: m_Type(EventType::WindowClosed)
		{
			XYZ_CORE_INFO("Window: Window closed");
		}

		virtual EventType GetEventType() const override { return m_Type; }

		static EventType GetStaticType() 
		{
			return EventType::WindowClosed;
		}

	private:
		EventType m_Type;
	};

}