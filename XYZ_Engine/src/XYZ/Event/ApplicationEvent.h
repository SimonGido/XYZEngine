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
			:m_Width(width), m_Height(height),m_Component(EventComponent::WindowResized)
		{
			XYZ_LOG_INFO("Window: Resized window width: ", width, " height: ", height);
		}


		virtual std::pair<int, int> GetWinSize() const { return { (int)m_Width, (int)m_Height }; }
		inline int GetWidth() const { return m_Width; }
		inline int GetHeight() const { return m_Height; }

		virtual EventComponent GetEventComponent() const override { return m_Component; }
		static EventComponent GetStaticComponent()
		{
			return EventComponent::WindowResized;
		}

	private:
		int m_Width;
		int m_Height;
		EventComponent m_Component;
	};

	/*! @class WindowCloseEvent
	*	@brief Occurs when a window is closed
	*/
	class WindowCloseEvent : public Event
	{
	public:
		WindowCloseEvent()
			: m_Component(EventComponent::WindowClosed)
		{
			XYZ_LOG_INFO("Window: Window closed");
		}

		virtual EventComponent GetEventComponent() const override { return m_Component; }

		static EventComponent GetStaticComponent() 
		{
			return EventComponent::WindowClosed;
		}

	private:
		EventComponent m_Component;
	};

}