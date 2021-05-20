#pragma once
#include "InGuiWindow.h"

namespace XYZ {
	
	class InGuiBehavior
	{
	public:
		static void ButtonBehavior(const InGuiRect& rect, InGuiID id, uint8_t& result);
		static void SliderBehavior(const InGuiRect& rect, const InGuiRect& handleRect, InGuiID id, uint8_t& result);
		static void FloatBehavior(const InGuiRect& rect, InGuiID id, uint8_t& result, float& value, int32_t decimalPrecision);
		static void IntBehavior(const InGuiRect& rect, InGuiID id, uint8_t& result, int32_t& value);
		static void UIntBehavior(const InGuiRect& rect, InGuiID id, uint8_t& result, uint32_t& value);

	private:
		template <typename T, typename ...Args>
		static void inputSelectionBehavior(InGuiID id, T& value, Args ... args);

		static void inputBehavior(InGuiID id);
	};

}