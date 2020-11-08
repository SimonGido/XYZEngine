#include "stdafx.h"
#include "Checkbox.h"


namespace XYZ {
	Checkbox::Checkbox()
	{
	}
	void Checkbox::OnUpdate(Timestep ts)
	{
		if (m_Checked)
			Execute(CheckedEvent{});
	}
	void Checkbox::OnEvent(Event& event)
	{

		EventDispatcher dispatcher(event);
		if (event.GetEventType() == EventType::Click)
		{
			m_Checked = !m_Checked;
			if (m_Checked)
				event.Handled = Execute(CheckedEvent{});
			else
				event.Handled = Execute(UnCheckedEvent{});
		}	
		else if (event.GetEventType() == EventType::Hoover)
		{
			event.Handled = Execute(HooverEvent{});
			m_Hoovered = true;
		}
		else if (event.GetEventType() == EventType::UnHoover)
		{
			event.Handled = Execute(UnHooverEvent{});
			m_Hoovered = false;
		}
	}
}