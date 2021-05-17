#include "stdafx.h"
#include "InGuiBehavior.h"

#include "InGui.h"

namespace XYZ {

	void InGuiBehavior::ButtonBehavior(const InGuiRect& rect, InGuiID id, uint8_t& result)
	{
		InGuiContext& context = InGui::GetContext();
		const InGuiInput& input = context.m_Input;
		const InGuiFrame& frame = context.m_FrameData;
		const InGuiWindow* currentWindow = frame.CurrentWindow;
		XYZ_ASSERT(currentWindow, "Current window is nullptr");

		if (rect.Overlaps(frame.MousePosition) && currentWindow->IsFocused())
		{
			result |= InGui::Hoover;
			if (IS_SET(input.Flags, InGuiInput::LeftMouseButtonPressed)
			&& !context.m_LastLeftPressedID)
			{
				result |= InGui::Active;
				context.m_LastLeftPressedID = id;
			}
		}
	}
	void InGuiBehavior::SliderBehavior(const InGuiRect& rect, const InGuiRect& handleRect, InGuiID id, uint8_t& result)
	{
		InGuiContext& context = InGui::GetContext();
		const InGuiInput& input = context.m_Input;
		const InGuiFrame& frame = context.m_FrameData;
		const InGuiWindow* currentWindow = frame.CurrentWindow;
		XYZ_ASSERT(currentWindow, "Current window is nullptr");

		if (context.m_LastLeftPressedID == id)
		{
			result |= InGui::Active;
		}
		if (rect.Overlaps(frame.MousePosition) && currentWindow->IsFocused())
		{
			result |= InGui::Hoover;
			if (IS_SET(input.Flags, InGuiInput::LeftMouseButtonPressed) 
			&& handleRect.Overlaps(frame.MousePosition)
			&& !context.m_LastLeftPressedID)
			{
				result |= InGui::Active;
				context.m_LastLeftPressedID = id;
			}
		}
	}
}