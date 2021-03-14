#pragma once
#include "XYZ/Event/InputEvent.h"
#include "InGuiAllocator.h"

namespace XYZ {

	class IGContext;
	class IGInput
	{
	public:
		bool OnMouseButtonPress(MouseButtonPressEvent& e, IGContext& context);
		bool OnMouseButtonRelease(MouseButtonReleaseEvent& e, IGContext& context);
		bool OnMouseMove(MouseMovedEvent& e, IGContext& context);
		bool OnMouseScroll(MouseScrollEvent& e, IGContext& context);
	};
}