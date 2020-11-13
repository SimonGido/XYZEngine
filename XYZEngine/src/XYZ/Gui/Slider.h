#pragma once
#include "XYZ/ECS/Component.h"
#include "XYZ/Event/GuiEvent.h"
#include "XYZ/Event/EventSystem.h"


namespace XYZ {


	class Slider : public EventSystem<ClickEvent, ReleaseEvent, HooverEvent, UnHooverEvent>,
		public ECS::Type<Slider>
	{
	public:
		Slider() = default;
		void OnEvent(Event& event);

	private:
		float m_Value = 0.0f;
	};
}