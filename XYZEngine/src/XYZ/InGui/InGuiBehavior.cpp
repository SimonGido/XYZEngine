#include "stdafx.h"
#include "InGuiBehavior.h"

#include "InGui.h"
#include "InGuiUtil.h"

namespace XYZ {

	void InGuiBehavior::ButtonBehavior(const InGuiRect& rect, InGuiID id, uint8_t& result)
	{
		InGuiContext& context = InGui::GetContext();
		const InGuiInput& input = context.m_Input;
		const InGuiFrame& frame = context.m_FrameData;
		const InGuiWindow* currentWindow = frame.CurrentWindow;
		XYZ_ASSERT(currentWindow, "Current window is nullptr");

		if (rect.Overlaps(input.MousePosition) && currentWindow->IsFocused() && (!context.m_LastHooveredID || context.m_LastHooveredID == id))
		{
			result |= InGui::Hoover;
			context.m_LastHooveredID = id;
			if (IS_SET(input.Flags, InGuiInput::LeftMouseButtonPressed)
			&& !context.m_LastLeftPressedID)
			{
				result |= InGui::Pressed;
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
			result |= InGui::Pressed;
		}
		if (rect.Overlaps(input.MousePosition) && currentWindow->IsFocused() && (!context.m_LastHooveredID || context.m_LastHooveredID == id))
		{
			result |= InGui::Hoover;
			context.m_LastHooveredID = id;
			if (IS_SET(input.Flags, InGuiInput::LeftMouseButtonPressed) 
			&& handleRect.Overlaps(input.MousePosition)
			&& !context.m_LastLeftPressedID)
			{
				result |= InGui::Pressed;
				context.m_LastLeftPressedID = id;
			}
		}
	}
	void InGuiBehavior::InputBehavior(const InGuiRect& rect, InGuiID id, uint8_t& result, float& value, int decimalPrecision)
	{
		InGuiContext& context = InGui::GetContext();
		InGuiInput& input = context.m_Input;
		const InGuiFrame& frame = context.m_FrameData;
		const InGuiWindow* currentWindow = frame.CurrentWindow;
		XYZ_ASSERT(currentWindow, "Current window is nullptr");

		if (rect.Overlaps(input.MousePosition) && currentWindow->IsFocused() && (!context.m_LastHooveredID || context.m_LastHooveredID == id))
		{
			result |= InGui::Hoover;
			context.m_LastHooveredID = id;
			if (IS_SET(input.Flags, InGuiInput::LeftMouseButtonPressed)
				&& !context.m_LastLeftPressedID
				)
			{
				context.m_LastLeftPressedID = id;
				if (context.m_LastInputID != id)
				{
					result |= InGui::Pressed;
					context.m_LastInputID = id;

					char buffer[InGuiContext::sc_InputValueBufferSize];
					Util::FormatFloat(buffer, InGuiContext::sc_InputValueBufferSize, value, decimalPrecision);
					context.m_TemporaryTextBuffer = buffer;
				}
				else
				{
					value = (float)std::atof(context.m_TemporaryTextBuffer.c_str());
					context.m_LastInputID = 0;
				}
			}
		}
		if (context.m_LastInputID == id)
		{
			if (input.KeyPressed == (int)KeyCode::KEY_BACKSPACE)
			{
				if (!context.m_TemporaryTextBuffer.empty())
					context.m_TemporaryTextBuffer.pop_back();
			}
			if (input.KeyTyped != InGuiInput::InvalidKey)
				context.m_TemporaryTextBuffer.push_back((char)input.KeyTyped);

			input.KeyPressed = InGuiInput::InvalidKey;
			input.KeyTyped	 = InGuiInput::InvalidKey;
		}
	}
}