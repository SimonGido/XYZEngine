#pragma once
#include "InGuiWindow.h"

namespace XYZ {
	
	class InGuiBehavior
	{
	public:
		static void ButtonBehavior(const InGuiRect& rect, InGuiID id, uint8_t& result);
		static void SliderBehavior(const InGuiRect& rect, const InGuiRect& handleRect, InGuiID id, uint8_t& result);
		static void InputBehavior(const InGuiRect& rect, InGuiID id, uint8_t& result, float& value, int decimalPrecision);
	};

}