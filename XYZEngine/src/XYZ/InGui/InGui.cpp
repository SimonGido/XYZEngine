#include "stdafx.h"
#include "InGui.h"

#include "InGuiUtil.h"
#include "InGuiBehavior.h"

#include "XYZ/Core/Input.h"

namespace XYZ {
	static InGuiContext s_Context;
	
	void InGui::BeginFrame()
	{
		auto [mx, my] = Input::GetMousePosition();
		s_Context.m_Input.MousePosition = { mx, my };	
	}

	void InGui::EndFrame()
	{
		s_Context.Render();
	}

	bool InGui::Begin(const char* name, InGuiWindowFlags flags)
	{
		InGuiInput& input = s_Context.m_Input;
		InGuiFrame& frame = s_Context.m_FrameData;
		InGuiConfig& config = s_Context.m_Config;

		XYZ_ASSERT(!frame.CurrentWindow, "Missing end call");
		InGuiWindow* window = s_Context.GetWindow(name);
		if (!window)
			window = s_Context.CreateWindow(name);
		window->StyleFlags = flags;

		frame.CurrentWindow = window;
		window->DrawList.Clear();
		glm::vec4 color = config.Colors[InGuiConfig::WindowDefault];
		if (window->HandleHoover(input.MousePosition))
			color = config.Colors[InGuiConfig::WindowHoover];		
		window->HandleResize(input.MousePosition);
		window->HandleMove(input.MousePosition, frame.MovedWindowOffset);
	
		window->PushItselfToDrawlist(color);
		if (IS_SET(flags, InGuiWindowStyleFlags::ScrollEnabled))
			window->HandleScrollbars();
		window->FrameData = InGuiWindowFrameData(window);
		return !IS_SET(window->EditFlags, InGuiWindowEditFlags::Collapsed);
	}
	void InGui::End()
	{
		XYZ_ASSERT(s_Context.m_FrameData.CurrentWindow, "Missing begin call");
		s_Context.m_FrameData.CurrentWindow = nullptr;
	}

	void InGui::Separator()
	{
		InGuiWindow* window = s_Context.m_FrameData.CurrentWindow;
		if (!window->IsActive || IS_SET(window->EditFlags, InGuiWindowEditFlags::Collapsed))
			return;

		const InGuiConfig& config = s_Context.m_Config;
		const glm::vec2& padding = config.WindowPadding;
		if (window->Axis == AxisPlacement::Horizontal)
		{
			window->FrameData.CursorPos.y += window->FrameData.Params.RowHeight + padding.y;
			window->FrameData.CursorPos.x = window->Position.x + padding.x;
			window->FrameData.Params.RowHeight = 0;
		}
		else
		{
			window->FrameData.CursorPos.x += window->FrameData.Params.RowWidth + padding.x;
			window->FrameData.CursorPos.y = window->Position.y + config.PanelHeight + padding.y;
			window->FrameData.Params.RowWidth = 0;
		}
	}

	bool InGui::BeginMenuBar()
	{
		XYZ_ASSERT(!s_Context.m_MenuBarActive, "Menu bar is already active, forgot end menu bar");
		InGuiWindow* window = s_Context.m_FrameData.CurrentWindow;
		if (!window->IsActive || IS_SET(window->EditFlags, InGuiWindowEditFlags::Collapsed))
			return false;

		if (!IS_SET(window->StyleFlags, InGuiWindowStyleFlags::MenuEnabled))
			return false;

		s_Context.m_MenuBarActive = true;
		const InGuiConfig& config = s_Context.m_Config;
		const glm::vec4 color = config.Colors[InGuiConfig::Button];
		const glm::vec2 pos = { window->Position.x , window->Position.y + config.PanelHeight };
		const glm::vec2 size = { window->Size.x, config.MenuBarHeight };
		window->PushQuadNotClipped(
			color, config.SubTextures[InGuiConfig::Button]->GetTexCoords(),
			pos, size
		);
		window->SetCursorPosition(pos);
		return true;
	}

	void InGui::EndMenuBar()
	{
		XYZ_ASSERT(s_Context.m_MenuBarActive, "Menu bar is not active, forgot begin menu bar");
		InGuiWindow* window = s_Context.m_FrameData.CurrentWindow;
		if (!window->IsActive || IS_SET(window->EditFlags, InGuiWindowEditFlags::Collapsed))
			return;
		window->FrameData = InGuiWindowFrameData(window);
		s_Context.m_MenuBarActive = false;
	}

	void InGui::Group()
	{
		InGuiWindow* window = s_Context.m_FrameData.CurrentWindow;
		if (!window->IsActive || IS_SET(window->EditFlags, InGuiWindowEditFlags::Collapsed))
			return;
		window->Grouping = true;
	}

	void InGui::EndGroup()
	{
		InGuiWindow* window = s_Context.m_FrameData.CurrentWindow;
		if (!window->IsActive || IS_SET(window->EditFlags, InGuiWindowEditFlags::Collapsed))
			return;
		window->Grouping = false;
	}

	uint8_t InGui::BeginMenu(const char* label, float width)
	{		
		InGuiWindow* window = s_Context.m_FrameData.CurrentWindow;
		if (!window->IsActive || IS_SET(window->EditFlags, InGuiWindowEditFlags::Collapsed))
			return 0;
		
		InGuiFrame& frame = s_Context.m_FrameData;
		const InGuiInput& input = s_Context.m_Input;
		const InGuiConfig& config = s_Context.m_Config;
		const glm::vec2 labelSize = Util::CalculateTextSize(label, config.Font);

		;
		InGuiID id = window->GetID(label);
		uint8_t result = 0;
		const glm::vec2 size = { width, config.MenuBarHeight };
		const glm::vec2 pos = window->FrameData.CursorPos;
		window->FrameData.CursorPos.y += size.y;
		frame.CurrentMenuWidth = width;

		glm::vec4 color = config.Colors[InGuiConfig::MenuDefault];
		InGuiRect rect(pos, pos + size);
		InGuiBehavior::ButtonBehavior(rect, id, result);
		if (IS_SET(result, InGui::Hoover))
		{
			color = config.Colors[InGuiConfig::MenuHoover];
		}
		if (IS_SET(result, InGui::Pressed))
		{
			s_Context.m_MenuOpenID = (s_Context.m_MenuOpenID != id) ? id : 0;
		}
		if (s_Context.m_MenuOpenID == id)
			result |= InGui::Pressed;

		window->PushQuadOverlay(
			color, config.SubTextures[InGuiConfig::Button]->GetTexCoords(),
			pos, size
		);
		window->PushTextOverlay(label, config.Colors[InGuiConfig::TextColor], rect.Min, rect.Max, nullptr);
		return result;
	}

	void InGui::EndMenu()
	{
		InGuiWindow* window = s_Context.m_FrameData.CurrentWindow;
		if (!window->IsActive || IS_SET(window->EditFlags, InGuiWindowEditFlags::Collapsed))
			return;
		
		InGuiFrame& frame = s_Context.m_FrameData;
		const InGuiConfig& config = s_Context.m_Config;
		for (const InGuiID id : frame.MenuItems)
			window->FrameData.CursorPos.y -= config.MenuItemSize.y;
		frame.MenuItems.clear();
		window->FrameData.CursorPos.x += frame.CurrentMenuWidth;
		window->FrameData.CursorPos.y -= config.MenuBarHeight;
	}

	uint8_t InGui::MenuItem(const char* label)
	{
		InGuiWindow* window = s_Context.m_FrameData.CurrentWindow;
		if (!window->IsActive || IS_SET(window->EditFlags, InGuiWindowEditFlags::Collapsed))
			return 0;

		uint8_t result = 0;
		InGuiFrame& frame = s_Context.m_FrameData;
		const InGuiInput& input = s_Context.m_Input;
		const InGuiConfig& config = s_Context.m_Config;
		const glm::vec2 labelSize = Util::CalculateTextSize(label, config.Font);

		InGuiID id = window->GetID(label);
		frame.MenuItems.push_back(id);

		const glm::vec2 pos = window->FrameData.CursorPos;
		window->FrameData.CursorPos.y += config.MenuItemSize.y;

		glm::vec4 color = config.Colors[InGuiConfig::MenuDefault];
		InGuiRect rect(pos, pos + config.MenuItemSize);
		InGuiBehavior::ButtonBehavior(rect, id, result);
		if (IS_SET(result, InGui::Hoover))
		{
			color = config.Colors[InGuiConfig::ButtonHoover];
		}
		if (IS_SET(result, InGui::Pressed))
		{
			s_Context.m_MenuOpenID = 0;
		}
		window->PushQuadNotClippedOverlay(
			color, config.SubTextures[InGuiConfig::Button]->GetTexCoords(),
			pos, config.MenuItemSize
		);
		window->PushTextNotClippedOverlay(label, config.Colors[InGuiConfig::TextColor], rect.Min, rect.Max, nullptr);
		return result;
	}

	uint8_t InGui::Button(const char* label, const glm::vec2& size)
	{
		InGuiWindow* window = s_Context.m_FrameData.CurrentWindow;
		if (!window->IsActive || IS_SET(window->EditFlags, InGuiWindowEditFlags::Collapsed))
			return 0;

		uint8_t result = 0;
		const InGuiInput& input = s_Context.m_Input;
		const InGuiFrame& frame = s_Context.m_FrameData;
		const InGuiConfig& config = s_Context.m_Config;
		const glm::vec2 labelSize = Util::CalculateTextSize(label, config.Font);
		if (window->NextWidgetClipped(size))
		{
			window->MoveCursorPosition(size);
			return 0;
		}
		InGuiID id = window->GetID(label);

		const glm::vec2 pos = window->MoveCursorPosition(size);
		glm::vec4 color = config.Colors[InGuiConfig::ButtonDefault];
		InGuiRect rect(pos, pos + size);
		InGuiBehavior::ButtonBehavior(rect, id, result);
		if (IS_SET(result, InGui::Hoover))
		{
			color = config.Colors[InGuiConfig::ButtonHoover];
		}
		window->PushQuad(
			color, config.SubTextures[InGuiConfig::Button]->GetTexCoords(),
			pos, size
		);
		window->PushTextClipped(label, color, rect.Min, rect.Max, nullptr);
		return result;
	}
	uint8_t InGui::Checkbox(const char* label, const glm::vec2& size, bool& checked)
	{
		InGuiWindow* window = s_Context.m_FrameData.CurrentWindow;
		if (!window->IsActive || IS_SET(window->EditFlags, InGuiWindowEditFlags::Collapsed))
			return 0;

		uint8_t result = 0;
		const InGuiInput& input = s_Context.m_Input;
		const InGuiFrame& frame = s_Context.m_FrameData;
		const InGuiConfig& config = s_Context.m_Config;
		const glm::vec2 labelSize = Util::CalculateTextSize(label, config.Font);
		const glm::vec2 fullSize = glm::vec2(size.x + labelSize.x, std::max(size.y, labelSize.y));
		if (window->NextWidgetClipped(fullSize))
		{
			window->MoveCursorPosition(fullSize);
			return 0;
		}
		InGuiID id = window->GetID(label);

		const glm::vec2 pos = window->MoveCursorPosition(fullSize);
		glm::vec4 color = config.Colors[InGuiConfig::CheckboxDefault];
		InGuiRect rect(pos, pos + size);
		InGuiBehavior::ButtonBehavior(rect, id, result);
		if (IS_SET(result, InGui::Hoover))
		{
			color = config.Colors[InGuiConfig::CheckboxHoover];
		}
		if (IS_SET(result, InGui::Pressed))
		{
			checked = !checked;
		}
		
		size_t subTextureIndex = checked ? InGuiConfig::CheckboxChecked : InGuiConfig::CheckboxUnChecked;
		window->PushQuad(
			color, config.SubTextures[subTextureIndex]->GetTexCoords(),
			pos, size
		);
		
		window->PushText(
			label, color, 
			rect.Min + glm::vec2(size.x, 0.0f), 
			rect.Max + glm::vec2(labelSize.x, 0.0f), 
			&labelSize
		);
		return result;
	}

	uint8_t InGui::SliderFloat(const char* label, const glm::vec2& size, float& value, float min, float max, const char* format)
	{
		InGuiWindow* window = s_Context.m_FrameData.CurrentWindow;
		if (!window->IsActive || IS_SET(window->EditFlags, InGuiWindowEditFlags::Collapsed))
			return 0;

		uint8_t result = 0;
		const InGuiInput& input = s_Context.m_Input;
		const InGuiFrame& frame = s_Context.m_FrameData;
		const InGuiConfig& config = s_Context.m_Config;
		const glm::vec2 labelSize = Util::CalculateTextSize(label, config.Font);
		const glm::vec2 fullSize = glm::vec2(size.x + labelSize.x, std::max(size.y, labelSize.y));
		if (window->NextWidgetClipped(fullSize))
		{
			window->MoveCursorPosition(fullSize);
			return 0;
		}
		InGuiID id = window->GetID(label);

		float diff = max - min;
		float scale = (value / diff);
		const glm::vec2 pos = window->MoveCursorPosition(fullSize);
		const glm::vec2 handleSize(size.y, size.y);
		const glm::vec2 handlePos = pos + glm::vec2((size.x - handleSize.x) * scale, 0.0f);

		glm::vec4 color = config.Colors[InGuiConfig::SliderDefault];
		InGuiRect rect(pos, pos + size);
		InGuiRect handleRect(handlePos, handlePos + handleSize);
		InGuiBehavior::SliderBehavior(rect, handleRect, id, result);
		if (IS_SET(result, InGui::Hoover))
		{
			color = config.Colors[InGuiConfig::SliderHoover];
		}
		if (IS_SET(result, InGui::Pressed))
		{
			float multiplier = (input.MousePosition.x - pos.x) / size.x;
			value = std::clamp(diff * multiplier, min, max);
		}

		window->PushQuad(
			color, config.SubTextures[InGuiConfig::Slider]->GetTexCoords(),
			pos, size
		);
		window->PushQuad(
			color, config.SubTextures[InGuiConfig::SliderHandle]->GetTexCoords(),
			handlePos, handleSize
		);
		window->PushText(
			label, color,
			rect.Min + glm::vec2(size.x, 0.0f),
			rect.Max + glm::vec2(labelSize.x, 0.0f),
			&labelSize
		);
		if (format)
		{
			char buffer[InGuiContext::sc_SliderValueBufferSize];
			sprintf(buffer, format, value);
			window->PushText(buffer, color, rect.Min, rect.Max, nullptr);
		}
		return result;
	}

	uint8_t InGui::VSliderFloat(const char* label, const glm::vec2& size, float& value, float min, float max, const char* format)
	{
		InGuiWindow* window = s_Context.m_FrameData.CurrentWindow;
		if (!window->IsActive || IS_SET(window->EditFlags, InGuiWindowEditFlags::Collapsed))
			return 0;

		uint8_t result = 0;
		const InGuiInput& input = s_Context.m_Input;
		const InGuiFrame& frame = s_Context.m_FrameData;
		const InGuiConfig& config = s_Context.m_Config;
		const glm::vec2 labelSize = Util::CalculateTextSize(label, config.Font);
		const glm::vec2 fullSize = glm::vec2(size.x + labelSize.x, std::max(size.y, labelSize.y));
		if (window->NextWidgetClipped(fullSize))
		{
			window->MoveCursorPosition(fullSize);
			return 0;
		}

		InGuiID id = window->GetID(label);

		float diff = max - min;
		float scale = (value / diff);
		const glm::vec2 pos = window->MoveCursorPosition(fullSize);
		const glm::vec2 handleSize(size.x, size.x);
		const glm::vec2 handlePos = pos + glm::vec2(0.0f, (size.y - handleSize.y) * scale);

		glm::vec4 color = config.Colors[InGuiConfig::SliderDefault];
		InGuiRect rect(pos, pos + size);
		InGuiRect handleRect(handlePos, handlePos + handleSize);
		InGuiBehavior::SliderBehavior(rect, handleRect, id, result);
		if (IS_SET(result, InGui::Hoover))
		{
			color = config.Colors[InGuiConfig::SliderHoover];
		}
		if (IS_SET(result, InGui::Pressed))
		{
			float multiplier = (input.MousePosition.y - pos.y) / size.y;
			value = std::clamp(diff * multiplier, min, max);
		}

		window->PushQuad(
			color, config.SubTextures[InGuiConfig::Slider]->GetTexCoords(),
			pos, size
		);
		window->PushQuad(
			color, config.SubTextures[InGuiConfig::SliderHandle]->GetTexCoords(),
			handlePos, handleSize
		);
		window->PushText(
			label, color,
			rect.Min + glm::vec2(size.x, 0.0f),
			rect.Max + glm::vec2(labelSize.x, 0.0f),
			&labelSize
		);
		if (format)
		{
			char buffer[InGuiContext::sc_SliderValueBufferSize];
			sprintf(buffer, format, value);
			window->PushText(buffer, color, rect.Min, rect.Max, nullptr);
		}
		return result;
	}

	uint8_t InGui::Float(const char* label, const glm::vec2& size, float& value, int32_t decimalPrecision)
	{
		InGuiWindow* window = s_Context.m_FrameData.CurrentWindow;
		if (!window->IsActive || IS_SET(window->EditFlags, InGuiWindowEditFlags::Collapsed))
			return 0;

		uint8_t result = 0;
		const InGuiInput& input = s_Context.m_Input;
		const InGuiFrame& frame = s_Context.m_FrameData;
		const InGuiConfig& config = s_Context.m_Config;
		const glm::vec2 labelSize = Util::CalculateTextSize(label, config.Font);
		const glm::vec2 fullSize = glm::vec2(size.x + labelSize.x, std::max(size.y, labelSize.y));
		if (window->NextWidgetClipped(fullSize))
		{
			window->MoveCursorPosition(fullSize);
			return 0;
		}
		InGuiID id = window->GetID(label);

		const glm::vec2 pos = window->MoveCursorPosition(fullSize);
		glm::vec4 color = config.Colors[InGuiConfig::ButtonDefault];
		InGuiRect rect(pos, pos + size);
		InGuiBehavior::FloatBehavior(rect, id, result, value, decimalPrecision);		
		if (IS_SET(result, InGui::Hoover | InGui::Pressed) || s_Context.m_LastInputID == id)
		{
			color = config.Colors[InGuiConfig::ButtonHoover];
		}

		window->PushQuad(
			color, config.SubTextures[InGuiConfig::Button]->GetTexCoords(),
			pos, size
		);
		window->PushText(
			label, color,
			rect.Min + glm::vec2(size.x, 0.0f),
			rect.Max + glm::vec2(labelSize.x, 0.0f),
			&labelSize
		);
			
		if (s_Context.m_LastInputID == id)
		{ 
			window->PushTextClipped(s_Context.m_TemporaryTextBuffer.c_str(), color, rect.Min, rect.Max, nullptr);
		}
		else
		{
			char buffer[InGuiContext::sc_InputValueBufferSize];
			Util::FormatFloat(buffer, InGuiContext::sc_InputValueBufferSize, value, decimalPrecision);
			window->PushTextClipped(buffer, color, rect.Min, rect.Max, nullptr);
		}		
		return result;
	}

	uint8_t InGui::Float2(const char* label1, const char*label2, const glm::vec2& size, float* values, int32_t decimalPrecision)
	{
		InGuiWindow* window = s_Context.m_FrameData.CurrentWindow;
		uint8_t result = Float(label1, size, values[0], decimalPrecision) | Float(label2, size, values[1], decimalPrecision);
		return result;
	}

	uint8_t InGui::Int(const char* label, const glm::vec2& size, int32_t& value)
	{
		return uint8_t();
	}

	uint8_t InGui::UInt(const char* label, const glm::vec2& size, uint32_t& value)
	{
		return uint8_t();
	}

	uint8_t InGui::String(const char* label, const glm::vec2& size, std::string& value)
	{
		return uint8_t();
	}
	
	InGuiContext& InGui::GetContext()
	{
		return s_Context;
	}
}