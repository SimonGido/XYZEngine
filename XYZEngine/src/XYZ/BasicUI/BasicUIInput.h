#pragma once
#include "XYZ/Event/InputEvent.h"

#include "BasicUI.h"
#include "BasicUIData.h"

namespace XYZ {

	class bUIInput
	{
	public:
		static bool OnMouseButtonPress(MouseButtonPressEvent& event, bUIEditData& editData, bUIData& data);
		static bool OnMouseButtonRelease(MouseButtonReleaseEvent& event, bUIEditData& editData, bUIData& data);
		static bool OnMouseMove(MouseMovedEvent& event, bUIEditData& editData, bUIData& data);
		static bool OnMouseScroll(MouseScrollEvent& event, bUIEditData& editData, bUIData& data);
		static bool OnKeyType(KeyTypedEvent& event);
		static bool OnKeyPress(KeyPressedEvent& event);
	};

}