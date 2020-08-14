#pragma once
#include "XYZ/ECS/Component.h"
#include "XYZ/Event/GuiEvent.h"
#include "XYZ/Event/EventSystem.h"
#include "Widget.h"


namespace XYZ {


	class Slider :  public Widget,
					public EventSystem<ClickEvent, ReleaseEvent, HooverEvent, UnHooverEvent>,
					public Type<Slider>
	{
	public:
		virtual void OnEvent(Event& event) override;

		virtual WidgetType GetWidgetType() override { return WidgetType::Button; }

	private:

		template <typename Event>
		bool executeEvent(Event& e)
		{
			Execute(e);
			return true;
		}

	private:
		float m_Value = 0.0f;
	};
}