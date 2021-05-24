#include "stdafx.h"
#include "InGuiBehavior.h"

#include "InGui.h"
#include "InGuiUtil.h"

namespace XYZ {

	template <>
	void InGuiBehavior::readValueFromBuffer(InGuiID id, float& value)
	{
		InGuiContext& context = InGui::GetContext();
		const InGuiInput& input = context.m_Input;
		if (context.m_LastInputID == id)
		{
			// If left mouse button is pressed, but does not overlap and still is capturing input, invalidate
			if (IS_SET(input.Flags, InGuiInput::LeftMouseButtonPressed))
			{
				value = (float)std::atof(context.m_TemporaryTextBuffer.c_str());
			}
		}		
	}


	template <>
	void InGuiBehavior::readValueFromBuffer(InGuiID id, int32_t& value)
	{
		InGuiContext& context = InGui::GetContext();
		const InGuiInput& input = context.m_Input;
		if (context.m_LastInputID == id)
		{
			// If left mouse button is pressed, but does not overlap and still is capturing input, invalidate
			if (IS_SET(input.Flags, InGuiInput::LeftMouseButtonPressed))
			{
				value = std::atoi(context.m_TemporaryTextBuffer.c_str());
			}
		}
	}

	template <>
	void InGuiBehavior::readValueFromBuffer(InGuiID id, uint32_t& value)
	{
		InGuiContext& context = InGui::GetContext();
		const InGuiInput& input = context.m_Input;
		if (context.m_LastInputID == id)
		{
			// If left mouse button is pressed, but does not overlap and still is capturing input, invalidate
			if (IS_SET(input.Flags, InGuiInput::LeftMouseButtonPressed))
			{
				value = std::atoi(context.m_TemporaryTextBuffer.c_str());
			}
		}
	}

	template <>
	void InGuiBehavior::readValueFromBuffer(InGuiID id, std::string& value)
	{
		InGuiContext& context = InGui::GetContext();
		const InGuiInput& input = context.m_Input;
		if (context.m_LastInputID == id)
		{
			// If left mouse button is pressed, but does not overlap and still is capturing input, invalidate
			if (IS_SET(input.Flags, InGuiInput::LeftMouseButtonPressed))
			{
				value = context.m_TemporaryTextBuffer;
			}
		}
	}

	template <>
	void InGuiBehavior::inputSelectionBehavior(InGuiID id, float& value, int32_t decimalPrecision)
	{
		InGuiContext& context = InGui::GetContext();
		if (context.m_LastInputID != id)
		{
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

	template <>
	void InGuiBehavior::inputSelectionBehavior(InGuiID id, int32_t& value)
	{
		InGuiContext& context = InGui::GetContext();
		if (context.m_LastInputID != id)
		{
			context.m_LastInputID = id;
			char buffer[InGuiContext::sc_InputValueBufferSize];
			sprintf(buffer, "%d", value);
			context.m_TemporaryTextBuffer = buffer;
		}
		else
		{
			value = std::atoi(context.m_TemporaryTextBuffer.c_str());
			context.m_LastInputID = 0;
		}
	}

	template <>
	void InGuiBehavior::inputSelectionBehavior(InGuiID id, uint32_t& value)
	{
		InGuiContext& context = InGui::GetContext();
		if (context.m_LastInputID != id)
		{
			context.m_LastInputID = id;
			char buffer[InGuiContext::sc_InputValueBufferSize];
			sprintf(buffer, "%u", value);
			context.m_TemporaryTextBuffer = buffer;
		}
		else
		{
			value = (uint32_t)std::atoi(context.m_TemporaryTextBuffer.c_str());
			context.m_LastInputID = 0;
		}
	}

	template <>
	void InGuiBehavior::inputSelectionBehavior(InGuiID id, std::string& value)
	{
		InGuiContext& context = InGui::GetContext();
		if (context.m_LastInputID != id)
		{
			context.m_LastInputID = id;
			context.m_TemporaryTextBuffer = value;
		}
		else
		{
			value = context.m_TemporaryTextBuffer;
			context.m_LastInputID = 0;
		}
	}

	void InGuiBehavior::ButtonBehavior(const InGuiRect& rect, InGuiID id, uint8_t& result)
	{
		InGuiContext& context = InGui::GetContext();
		const InGuiInput& input = context.m_Input;
		const InGuiFrame& frame = context.m_FrameData;
		const InGuiWindow* currentWindow = frame.CurrentWindow;
		XYZ_ASSERT(currentWindow, "Current window is nullptr");

		if (rect.Overlaps(input.MousePosition) 
		&& (currentWindow->IsFocused() || currentWindow->IsParentFocused())
		&& (!context.m_LastHooveredID  || context.m_LastHooveredID == id))
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
		if (rect.Overlaps(input.MousePosition) 
		&& (currentWindow->IsFocused() || currentWindow->IsParentFocused())
		&& (!context.m_LastHooveredID || context.m_LastHooveredID == id))
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
	void InGuiBehavior::FloatBehavior(const InGuiRect& rect, InGuiID id, uint8_t& result, float& value, int32_t decimalPrecision)
	{
		InGuiContext& context = InGui::GetContext();
		InGuiInput& input = context.m_Input;
		const InGuiFrame& frame = context.m_FrameData;
		const InGuiWindow* currentWindow = frame.CurrentWindow;
		XYZ_ASSERT(currentWindow, "Current window is nullptr");

		if (rect.Overlaps(input.MousePosition) 
		&& (currentWindow->IsFocused() || currentWindow->IsParentFocused())
		&& (!context.m_LastHooveredID  || context.m_LastHooveredID == id))
		{
			result |= InGui::Hoover;
			context.m_LastHooveredID = id;
			if (IS_SET(input.Flags, InGuiInput::LeftMouseButtonPressed)
				&& !context.m_LastLeftPressedID
				)
			{
				result |= InGui::Pressed;
				context.m_LastLeftPressedID = id;
				inputSelectionBehavior(id, value, decimalPrecision);
			}
		}
		readValueFromBuffer(id, value);
		inputBehavior(id);
	}

	void InGuiBehavior::IntBehavior(const InGuiRect& rect, InGuiID id, uint8_t& result, int32_t& value)
	{
		InGuiContext& context = InGui::GetContext();
		InGuiInput& input = context.m_Input;
		const InGuiFrame& frame = context.m_FrameData;
		const InGuiWindow* currentWindow = frame.CurrentWindow;
		XYZ_ASSERT(currentWindow, "Current window is nullptr");

		if (rect.Overlaps(input.MousePosition) 
		&& (currentWindow->IsFocused() || currentWindow->IsParentFocused())
		&& (!context.m_LastHooveredID || context.m_LastHooveredID == id))
		{
			result |= InGui::Hoover;
			context.m_LastHooveredID = id;
			if (IS_SET(input.Flags, InGuiInput::LeftMouseButtonPressed)
				&& !context.m_LastLeftPressedID
				)
			{
				result |= InGui::Pressed;
				context.m_LastLeftPressedID = id;
				inputSelectionBehavior(id, value);
			}
		}
		readValueFromBuffer(id, value);
		inputBehavior(id);
	}

	void InGuiBehavior::UIntBehavior(const InGuiRect& rect, InGuiID id, uint8_t& result, uint32_t& value)
	{
		InGuiContext& context = InGui::GetContext();
		InGuiInput& input = context.m_Input;
		const InGuiFrame& frame = context.m_FrameData;
		const InGuiWindow* currentWindow = frame.CurrentWindow;
		XYZ_ASSERT(currentWindow, "Current window is nullptr");

		if (rect.Overlaps(input.MousePosition) 
		&& (currentWindow->IsFocused() || currentWindow->IsParentFocused())
		&& (!context.m_LastHooveredID  || context.m_LastHooveredID == id))
		{
			result |= InGui::Hoover;
			context.m_LastHooveredID = id;
			if (IS_SET(input.Flags, InGuiInput::LeftMouseButtonPressed)
				&& !context.m_LastLeftPressedID
				)
			{
				result |= InGui::Pressed;
				context.m_LastLeftPressedID = id;
				inputSelectionBehavior(id, value);
			}
		}
		readValueFromBuffer(id, value);
		inputBehavior(id);
	}

	void InGuiBehavior::StringBehavior(const InGuiRect& rect, InGuiID id, uint8_t& result, std::string& value)
	{
		InGuiContext& context = InGui::GetContext();
		InGuiInput& input = context.m_Input;
		const InGuiFrame& frame = context.m_FrameData;
		const InGuiWindow* currentWindow = frame.CurrentWindow;
		XYZ_ASSERT(currentWindow, "Current window is nullptr");

		if (rect.Overlaps(input.MousePosition)
			&& (currentWindow->IsFocused() || currentWindow->IsParentFocused())
			&& (!context.m_LastHooveredID || context.m_LastHooveredID == id))
		{
			result |= InGui::Hoover;
			context.m_LastHooveredID = id;
			if (IS_SET(input.Flags, InGuiInput::LeftMouseButtonPressed)
				&& !context.m_LastLeftPressedID
				)
			{
				result |= InGui::Pressed;
				context.m_LastLeftPressedID = id;
				inputSelectionBehavior(id, value);
			}
		}
		readValueFromBuffer(id, value);
		inputBehavior(id);
	}

	void InGuiBehavior::inputBehavior(InGuiID id)
	{
		InGuiContext& context = InGui::GetContext();
		InGuiInput& input = context.m_Input;
		if (context.m_LastInputID == id)
		{
			// If left mouse button is pressed, but does not overlap and still is capturing input, invalidate
			if (IS_SET(input.Flags, InGuiInput::LeftMouseButtonPressed)
			 && context.m_LastLeftPressedID != id)
			{
				context.m_LastInputID = 0;
			}
			else if (input.KeyPressed == (int)KeyCode::KEY_BACKSPACE)
			{
				if (!context.m_TemporaryTextBuffer.empty())
					context.m_TemporaryTextBuffer.pop_back();
			}
			else if (input.KeyTyped != InGuiInput::InvalidKey)
				context.m_TemporaryTextBuffer.push_back((char)input.KeyTyped);

			// If got captured, invalidate keys
			input.KeyPressed = InGuiInput::InvalidKey;
			input.KeyTyped = InGuiInput::InvalidKey;
		}
	}
}