#pragma once
#include "XYZ/ECS/Component.h"
#include "XYZ/Event/Event.h"

namespace XYZ {

	enum class WidgetType
	{
		None,
		Button,
		Text,
		Slider,
		CheckBox,
		Image
	};

	class Widget
	{
	public:
		virtual ~Widget() = default;

		virtual void OnEvent(Event& event) {};
		virtual WidgetType GetWidgetType() { return WidgetType::None; }
	};
}