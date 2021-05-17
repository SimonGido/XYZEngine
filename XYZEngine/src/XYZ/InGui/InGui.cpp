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
		s_Context.m_FrameData.MousePosition = { mx, my };	
	}

	void InGui::EndFrame()
	{
		s_Context.Render();
	}

	bool InGui::Begin(const char* name)
	{
		InGuiFrame& frame = s_Context.m_FrameData;
		InGuiConfig& config = s_Context.m_Config;

		XYZ_ASSERT(!frame.CurrentWindow, "Missing end call");
		InGuiWindow* window = s_Context.GetWindow(name);
		if (!window)
			window = s_Context.CreateWindow(name);
		

		frame.CurrentWindow = window;
		window->DrawList.Clear();
		glm::vec4 color = config.Colors[InGuiConfig::DefaultColor];
		if (window->HandleHoover(frame.MousePosition))
			color = config.Colors[InGuiConfig::WindowHoover];		
		window->HandleResize(frame.MousePosition);
		window->HandleMove(frame.MousePosition, frame.MovedWindowOffset);
	
		window->PushItselfToDrawlist(color, config);
		window->HandleScrollbars();
		window->FrameData = InGuiWindowFrameData(window);
		return !IS_SET(window->Flags, InGuiWindow::Collapsed);
	}
	void InGui::End()
	{
		XYZ_ASSERT(s_Context.m_FrameData.CurrentWindow, "Missing begin call");
		s_Context.m_FrameData.CurrentWindow = nullptr;
	}

	void InGui::Separator()
	{
		InGuiWindow* window = s_Context.m_FrameData.CurrentWindow;
		if (!window->IsActive || IS_SET(window->Flags, InGuiWindow::Collapsed))
			return;

		const glm::vec2& padding = +InGui::GetContext().m_Config.WindowPadding;
		if (window->Axis == AxisPlacement::Horizontal)
		{
			window->FrameData.CursorPos.y += window->FrameData.Params.RowHeight + padding.y;
			window->FrameData.CursorPos.x = window->Position.x + padding.x;
			window->FrameData.Params.RowHeight = 0;
		}
		else
		{
			window->FrameData.CursorPos.x += window->FrameData.Params.RowWidth + padding.x;
			window->FrameData.CursorPos.y = window->Position.y + window->PanelHeight + padding.y;
			window->FrameData.Params.RowWidth = 0;
		}
	}

	uint8_t InGui::Button(const char* label, const glm::vec2& size)
	{
		InGuiWindow* window = s_Context.m_FrameData.CurrentWindow;
		if (!window->IsActive || IS_SET(window->Flags, InGuiWindow::Collapsed))
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
		std::hash<std::string_view> hasher;
		InGuiID id = hasher(label);

		const glm::vec2 pos = window->MoveCursorPosition(size);
		glm::vec4 color = config.Colors[InGuiConfig::DefaultColor];
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
		if (!window->IsActive || IS_SET(window->Flags, InGuiWindow::Collapsed))
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
		std::hash<std::string_view> hasher;
		InGuiID id = hasher(label);

		const glm::vec2 pos = window->MoveCursorPosition(fullSize);
		glm::vec4 color = config.Colors[InGuiConfig::DefaultColor];
		InGuiRect rect(pos, pos + size);
		InGuiBehavior::ButtonBehavior(rect, id, result);
		if (IS_SET(result, InGui::Hoover))
		{
			color = config.Colors[InGuiConfig::CheckboxHoover];
		}
		if (IS_SET(result, InGui::Active))
		{
			checked = !checked;
		}
			
		if (checked)
		{
			result |= InGui::Active;
			window->PushQuad(
				color, config.SubTextures[InGuiConfig::CheckboxChecked]->GetTexCoords(),
				pos, size
			);
		}
		else
		{
			result &= ~InGui::Active;
			window->PushQuad(
				color, config.SubTextures[InGuiConfig::CheckboxUnChecked]->GetTexCoords(),
				pos, size
			);
		}
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
		if (!window->IsActive || IS_SET(window->Flags, InGuiWindow::Collapsed))
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
		std::hash<std::string_view> hasher;
		InGuiID id = hasher(label);

		float diff = max - min;
		float scale = (value / diff);
		const glm::vec2 pos = window->MoveCursorPosition(fullSize);
		const glm::vec2 handleSize(size.y, size.y);
		const glm::vec2 handlePos = pos + glm::vec2((size.x - handleSize.x) * scale, 0.0f);

		glm::vec4 color = config.Colors[InGuiConfig::DefaultColor];
		InGuiRect rect(pos, pos + size);
		InGuiRect handleRect(handlePos, handlePos + handleSize);
		InGuiBehavior::SliderBehavior(rect, handleRect, id, result);
		if (IS_SET(result, InGui::Hoover))
		{
			color = config.Colors[InGuiConfig::SliderHoover];
		}
		if (IS_SET(result, InGui::Active))
		{
			float multiplier = (frame.MousePosition.x - pos.x) / size.x;
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
			char buffer[12];
			sprintf(buffer, format, value);
			window->PushText(buffer, color, rect.Min, rect.Max, nullptr);
		}
		return result;
	}

	uint8_t InGui::VSliderFloat(const char* label, const glm::vec2& size, float& value, float min, float max, const char* format)
	{
		InGuiWindow* window = s_Context.m_FrameData.CurrentWindow;
		if (!window->IsActive || IS_SET(window->Flags, InGuiWindow::Collapsed))
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

		std::hash<std::string_view> hasher;
		InGuiID id = hasher(label);

		float diff = max - min;
		float scale = (value / diff);
		const glm::vec2 pos = window->MoveCursorPosition(fullSize);
		const glm::vec2 handleSize(size.x, size.x);
		const glm::vec2 handlePos = pos + glm::vec2(0.0f, (size.y - handleSize.y) * scale);

		glm::vec4 color = config.Colors[InGuiConfig::DefaultColor];
		InGuiRect rect(pos, pos + size);
		InGuiRect handleRect(handlePos, handlePos + handleSize);
		InGuiBehavior::SliderBehavior(rect, handleRect, id, result);
		if (IS_SET(result, InGui::Hoover))
		{
			color = config.Colors[InGuiConfig::SliderHoover];
		}
		if (IS_SET(result, InGui::Active))
		{
			float multiplier = (frame.MousePosition.y - pos.y) / size.y;
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
			char buffer[12];
			sprintf(buffer, format, value);
			window->PushText(buffer, color, rect.Min, rect.Max, nullptr);
		}
		return result;
	}
	
	InGuiContext& InGui::GetContext()
	{
		return s_Context;
	}
}