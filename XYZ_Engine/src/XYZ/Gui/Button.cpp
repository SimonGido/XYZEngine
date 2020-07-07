#include "stdafx.h"
#include "Button.h"


namespace XYZ {
	Button::Button()
	{
	}

	void Button::OnEvent(Event& event)
	{
		EventDispatcher dispatcher(event);
		if (dispatcher.Dispatch<ClickEvent>(Hook(&Button::receiveEvent<ClickEvent>, this)))
		{
			std::cout << "Clicked" << std::endl;
			Execute(ClickEvent{});
		}
		else if (dispatcher.Dispatch<ReleaseEvent>(Hook(&Button::receiveEvent<ReleaseEvent>, this)))
		{
			std::cout << "Released" << std::endl;
			Execute(ReleaseEvent{});
		}
		else if (dispatcher.Dispatch<HooverEvent>(Hook(&Button::receiveEvent<HooverEvent>, this)))
		{
			std::cout << "Hoover" << std::endl;
			Execute(HooverEvent{});
		}
		else if (dispatcher.Dispatch<UnHooverEvent>(Hook(&Button::receiveEvent<UnHooverEvent>, this)))
		{
			std::cout << "UnHoover" << std::endl;
			Execute(UnHooverEvent{});
		}
	}
}