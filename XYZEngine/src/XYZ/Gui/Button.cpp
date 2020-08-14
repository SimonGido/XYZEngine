#include "stdafx.h"
#include "Button.h"


namespace XYZ {
	Button::Button()
	{
	}

	void Button::OnEvent(Event& event)
	{
		EventDispatcher dispatcher(event);
		if (dispatcher.Dispatch<ClickEvent>(Hook(&Button::executeEvent<ClickEvent>, this)))
		{
			std::cout << "Clicked" << std::endl;
		}
		else if (dispatcher.Dispatch<ReleaseEvent>(Hook(&Button::executeEvent<ReleaseEvent>, this)))
		{
			std::cout << "Released" << std::endl;
		}
		else if (dispatcher.Dispatch<HooverEvent>(Hook(&Button::executeEvent<HooverEvent>, this)))
		{
			std::cout << "Hoover" << std::endl;
		}
		else if (dispatcher.Dispatch<UnHooverEvent>(Hook(&Button::executeEvent<UnHooverEvent>, this)))
		{
			std::cout << "UnHoover" << std::endl;
		}
	}
}