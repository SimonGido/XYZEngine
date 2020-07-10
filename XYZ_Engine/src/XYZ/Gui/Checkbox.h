#pragma once

#include "XYZ/ECS/ECSManager.h"
#include "XYZ/Event/GuiEvent.h"
#include "XYZ/Event/EventSystem.h"
#include "Widget.h"

namespace XYZ {
	class Checkbox : public Widget,
				   public EventSystem<ClickEvent, ReleaseEvent, HooverEvent, UnHooverEvent, CheckedEvent>,
				   public Type<Checkbox>
	{
	public:
		Checkbox();

		virtual void OnUpdate(float dt) override;
		virtual void OnEvent(Event& event) override;
		virtual WidgetType GetWidgetType() override { return WidgetType::Button; }

	private:

		template <typename Event>
		bool executeEvent(Event& e)
		{
			return true;
		}

	private:
		bool m_Checked = false;
	};
}