#pragma once

#include "XYZ/ECS/ECSManager.h"
#include "XYZ/Event/GuiEvent.h"
#include "XYZ/Event/EventSystem.h"

#include "XYZ/Core/Timestep.h"

namespace XYZ {
	class Checkbox : public EventSystem<CheckedEvent, UnCheckedEvent, HooverEvent, UnHooverEvent>,
				     public Type<Checkbox>
	{
	public:
		Checkbox();

		void OnUpdate(Timestep ts);
		void OnEvent(Event& event);

	private:
		bool m_Checked = false;
		bool m_Hoovered = false;
	};
}