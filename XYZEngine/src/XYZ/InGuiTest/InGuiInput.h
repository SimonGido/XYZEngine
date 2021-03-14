#pragma once
#include "XYZ/Event/InputEvent.h"
#include "InGuiAllocator.h"

namespace XYZ {

	class IGInput
	{
	public:
		bool OnMouseButtonPress(MouseButtonPressEvent& e, IGAllocator& allocator);
		bool OnMouseButtonRelease(MouseButtonReleaseEvent& e, IGAllocator& allocator);
		bool OnMouseMove(MouseMovedEvent& e, IGAllocator& allocator);
		bool OnMouseScroll(MouseScrollEvent& e, IGAllocator& allocator);
	};
}