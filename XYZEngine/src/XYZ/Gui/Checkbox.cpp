#include "stdafx.h"
#include "Checkbox.h"


namespace XYZ {
	Checkbox::Checkbox()
	{
	}
	void Checkbox::OnUpdate(float dt)
	{
		if (m_Checked)
			Execute(CheckedEvent{});
	}
	void Checkbox::OnEvent(Event& event)
	{
		EventDispatcher dispatcher(event);
		if (dispatcher.Dispatch<ClickEvent>(Hook(&Checkbox::executeEvent<ClickEvent>, this)))
		{
			std::cout << "Clicked" << std::endl;
			m_Checked = !m_Checked;
			if (m_Checked)
				Execute(ClickEvent{});
			else
				Execute(ReleaseEvent{});
		}
		else if (dispatcher.Dispatch<HooverEvent>(Hook(&Checkbox::executeEvent<HooverEvent>, this)))
		{
			Execute(HooverEvent{});
		}
		else if (dispatcher.Dispatch<UnHooverEvent>(Hook(&Checkbox::executeEvent<UnHooverEvent>, this)))
		{
			Execute(UnHooverEvent{});
		}
	}
}