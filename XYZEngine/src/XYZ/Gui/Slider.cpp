#include "stdafx.h"
#include "Slider.h"


namespace XYZ {
	void Slider::OnEvent(Event& event)
	{
		EventDispatcher dispatcher(event);
		if (dispatcher.Dispatch<ClickEvent>(Hook(&Slider::executeEvent<ClickEvent>, this)))
		{
			std::cout << "Clicked" << std::endl;
		}
		else if (dispatcher.Dispatch<ReleaseEvent>(Hook(&Slider::executeEvent<ReleaseEvent>, this)))
		{
			std::cout << "Released" << std::endl;
		}
		else if (dispatcher.Dispatch<HooverEvent>(Hook(&Slider::executeEvent<HooverEvent>, this)))
		{
			std::cout << "Hoover" << std::endl;
		}
		else if (dispatcher.Dispatch<UnHooverEvent>(Hook(&Slider::executeEvent<UnHooverEvent>, this)))
		{
			std::cout << "UnHoover" << std::endl;
		}
	}
}