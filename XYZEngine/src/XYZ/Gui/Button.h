#pragma once
#include "XYZ/ECS/ECSManager.h"
#include "XYZ/Event/GuiEvent.h"
#include "XYZ/Event/EventSystem.h"
#include "Widget.h"

#include <glm/glm.hpp>

#include <functional>

namespace XYZ {
	class Button : public Widget,
				   public EventSystem<ClickEvent,ReleaseEvent,HooverEvent,UnHooverEvent>,
				   public Type<Button>
	{
	public:
		Button();


		virtual void OnEvent(Event& event) override;
		virtual WidgetType GetWidgetType() override { return WidgetType::Button; }

	private:

		template <typename Event>
		bool executeEvent(Event& e)
		{
			Execute(e);
			return true;
		}
	
	};
}