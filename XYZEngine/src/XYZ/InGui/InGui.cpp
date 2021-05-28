#include "stdafx.h"
#include "InGui.h"

#include "InGuiUtil.h"
#include "InGuiBehavior.h"
#include "InGuiSerializer.h"

#include "XYZ/Core/Input.h"

#include <stack>

namespace XYZ {
	static InGuiContext* s_Context = nullptr;
	
	void InGui::Init(const std::string& filepath)
	{
		if (!s_Context)
		{
			s_Context = new InGuiContext();
			if (!filepath.empty())
				InGuiSerializer::Deserialize(*s_Context, filepath);
		}
	}

	void InGui::Shutdown()
	{
		if (s_Context)
		{
			InGuiSerializer::Serialize(*s_Context, "InGui.yaml");
			InGui::DestroyDockSpace();
			delete s_Context;
		}
	}

	void InGui::InitDockSpace()
	{
		if (!s_Context->m_DockSpace.IsInitialized())
			s_Context->m_DockSpace.Init(glm::vec2(0.0f), { s_Context->m_ViewportWidth, s_Context->m_ViewportHeight });
	}

	void InGui::DestroyDockSpace()
	{
		s_Context->m_DockSpace.Destroy();
	}

	void InGui::BeginFrame()
	{
		auto [mx, my] = Input::GetMousePosition();
		s_Context->m_Input.MousePosition = { mx, my };	
	}

	void InGui::EndFrame()
	{
		s_Context->Render();
	}

	bool InGui::Begin(const char* name, InGuiWindowFlags flags)
	{
		InGuiInput& input = s_Context->m_Input;
		InGuiFrame& frame = s_Context->m_FrameData;
		InGuiConfig& config = s_Context->m_Config;

		InGuiWindow* window = s_Context->GetInGuiWindow(name);
		if (!window)
			window = s_Context->CreateInGuiWindow(name);
		window->StyleFlags = flags;
		window->ClipID = window->WorkClipID;
	
		InGuiClipID clipID = 0;
		if (!frame.WindowQueue.empty())
		{
			InGuiWindow* parent = frame.WindowQueue.back();
			window->SetParent(parent);
			clipID = parent->ClipID;		
		}
		else
			window->SetParent(nullptr);

		frame.CurrentWindow = window;
		window->DrawList.Clear();
		bool highlight = false;
		if (window->HandleHoover(input.MousePosition) || window->IsFocused())
			highlight = true;
		window->HandleResize(input.MousePosition);
		window->HandleMove(input.MousePosition, frame.MovedWindowOffset);
	
		window->PushItselfToDrawlist(highlight, clipID);
		if (IS_SET(flags, InGuiWindowStyleFlags::ScrollEnabled))
			window->HandleScrollbars();
		window->FrameData = InGuiWindowFrameData(window);
		
		frame.WindowQueue.push_back(window);
		return !IS_SET(window->EditFlags, InGuiWindowEditFlags::Collapsed);
	}
	void InGui::End()
	{
		XYZ_ASSERT(s_Context->m_FrameData.CurrentWindow, "Missing begin call");
		InGuiFrame& frame = s_Context->m_FrameData;
		frame.WindowQueue.pop_back();
		if (!frame.WindowQueue.empty())
		{
			frame.CurrentWindow = frame.WindowQueue.back();
		}
		else
		{
			frame.CurrentWindow = nullptr;
		}
	}

	void InGui::Separator()
	{
		InGuiWindow* window = s_Context->m_FrameData.CurrentWindow;
		if (!window->IsActive || IS_SET(window->EditFlags, InGuiWindowEditFlags::Collapsed))
			return;

		const InGuiConfig& config = s_Context->m_Config;
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

	InGuiID InGui::GetID(const char* name)
	{
		std::hash<std::string_view> hasher;
		return hasher(name);
	}

	void InGui::FocusWindow(const char* name)
	{
		s_Context->FocusWindow(s_Context->GetInGuiWindow(name));
	}

	void InGui::DockSpace()
	{
		InGuiDockSpace& dockSpace = s_Context->m_DockSpace;
		if (!dockSpace.IsInitialized())
			return;

		dockSpace.Drawlist.Clear();
		const InGuiConfig& config = s_Context->m_Config;
		const InGuiFrame& frame = s_Context->m_FrameData;
		const InGuiInput& input = s_Context->m_Input;
		
		InGuiWindow* focused = s_Context->m_FocusedWindow;
		if (dockSpace.ResizedNode)
			dockSpace.ResizedNode->HandleResize(input.MousePosition);

		std::stack<InGuiDockNode*> nodes;
		nodes.push(dockSpace.Root);
		while (!nodes.empty())
		{
			InGuiDockNode* tmp = nodes.top();
			nodes.pop();
			tmp->Update();
			if (tmp->Split != SplitType::None)
			{
				nodes.push(tmp->Children[0]);
				nodes.push(tmp->Children[1]);
			}
			else if (focused && IS_SET(focused->EditFlags, InGuiWindowEditFlags::Moving)
							 && IS_SET(focused->StyleFlags, InGuiWindowStyleFlags::DockingEnabled))
			{
				dockSpace.PushNodeRectangle(tmp->MiddleRect());
				dockSpace.PushNodeRectangle(tmp->LeftRect());
				dockSpace.PushNodeRectangle(tmp->RightRect());
				dockSpace.PushNodeRectangle(tmp->TopRect());
				dockSpace.PushNodeRectangle(tmp->BottomRect());
			}					
		}	
	}

	bool InGui::BeginMenuBar()
	{
		XYZ_ASSERT(!s_Context->m_MenuBarActive, "Menu bar is already active, forgot end menu bar");
		InGuiWindow* window = s_Context->m_FrameData.CurrentWindow;
		if (!window->IsActive || IS_SET(window->EditFlags, InGuiWindowEditFlags::Collapsed))
			return false;

		if (!IS_SET(window->StyleFlags, InGuiWindowStyleFlags::MenuEnabled))
			return false;

		s_Context->m_MenuBarActive = true;
		const InGuiConfig& config = s_Context->m_Config;
		const glm::vec4 color = config.Colors[InGuiConfig::MenuColor];
		const glm::vec2 pos = { window->Position.x, window->Position.y + config.PanelHeight };
		const glm::vec2 size = { window->Size.x, config.MenuBarHeight };
		window->PushQuadNotClipped(
			color, config.WhiteSubTexture->GetTexCoords(),
			pos, size, InGuiConfig::WhiteTextureIndex
		);
		window->SetCursorPosition(pos);
		return true;
	}

	void InGui::EndMenuBar()
	{
		XYZ_ASSERT(s_Context->m_MenuBarActive, "Menu bar is not active, forgot begin menu bar");
		InGuiWindow* window = s_Context->m_FrameData.CurrentWindow;
		if (!window->IsActive || IS_SET(window->EditFlags, InGuiWindowEditFlags::Collapsed))
			return;
		window->FrameData = InGuiWindowFrameData(window);
		s_Context->m_MenuBarActive = false;
	}


	void InGui::BeginGroup()
	{
		InGuiWindow* window = s_Context->m_FrameData.CurrentWindow;
		if (!window->IsActive || IS_SET(window->EditFlags, InGuiWindowEditFlags::Collapsed))
			return;
		window->Grouping = true;
	}

	void InGui::EndGroup()
	{
		InGuiWindow* window = s_Context->m_FrameData.CurrentWindow;
		if (!window->IsActive || IS_SET(window->EditFlags, InGuiWindowEditFlags::Collapsed))
			return;
		window->EndGroup();
	}

	bool InGui::BeginTab(const char* label)
	{
		InGuiWindow* window = s_Context->m_FrameData.CurrentWindow;
		if (!window->IsActive || IS_SET(window->EditFlags, InGuiWindowEditFlags::Collapsed))
			return 0;

		uint8_t result = 0;
		const InGuiInput& input = s_Context->m_Input;
		const InGuiFrame& frame = s_Context->m_FrameData;
		const InGuiConfig& config = s_Context->m_Config;
		const glm::vec2 labelSize = Util::CalculateTextSize(label, config.Font);
		const glm::vec2 fullSize = glm::vec2(labelSize.x + config.TabLabelOffset, config.PanelHeight);
		if (window->NextTabClipped())
		{
			window->MoveTabPosition(fullSize.x);
			return 0;
		}
		InGuiID id = window->GetID(label);

		const glm::vec2 pos(window->MoveTabPosition(fullSize.x), window->Position.y); 
		glm::vec4 color = config.Colors[InGuiConfig::TabColor];
		InGuiRect rect(pos, pos + fullSize);
		InGuiRect textRect(rect);
		rect.Union(window->PanelRect());

		InGuiBehavior::ButtonBehavior(rect, id, result);
		if (IS_SET(result, InGui::Hoover) || window->TabID == id)
		{
			color = config.Colors[InGuiConfig::TabHighlight];
		}
		if (IS_SET(result, InGui::Pressed))
		{
			window->TabID = id;
		}

		window->ClipID = window->PanelClipID;
		window->PushQuad(
			color, config.WhiteSubTexture->GetTexCoords(),
			pos, fullSize, InGuiConfig::WhiteTextureIndex
		);
		window->PushTextClipped(label, config.Colors[InGuiConfig::TextColor], textRect.Min, textRect.Max, nullptr);
		window->ClipID = window->WorkClipID;
		return (window->TabID == id);
	}

	void InGui::EnableHighlight()
	{
		s_Context->m_FrameData.HighlightNext = true;
	}

	void InGui::DisableHighlight()
	{
		s_Context->m_FrameData.HighlightNext = false;
	}

	void InGui::BeginTreeChild()
	{
		InGuiWindow* window = s_Context->m_FrameData.CurrentWindow;
		if (!window->IsActive || IS_SET(window->EditFlags, InGuiWindowEditFlags::Collapsed))
			return;
		InGuiFrame& frame = s_Context->m_FrameData;
		frame.CurrentTreeDepth++;
		return;
	}

	void InGui::EndTreeChild()
	{
		InGuiWindow* window = s_Context->m_FrameData.CurrentWindow;
		if (!window->IsActive || IS_SET(window->EditFlags, InGuiWindowEditFlags::Collapsed))
			return;
		InGuiFrame& frame = s_Context->m_FrameData;
		frame.CurrentTreeDepth--;
	}

	uint8_t InGui::TreeNode(const char* label, const glm::vec2& size, bool & open)
	{
		InGuiWindow* window = s_Context->m_FrameData.CurrentWindow;
		if (!window->IsActive || IS_SET(window->EditFlags, InGuiWindowEditFlags::Collapsed))
			return 0;

		uint8_t result = 0;
		const InGuiInput& input = s_Context->m_Input;
		const InGuiFrame& frame = s_Context->m_FrameData;
		const InGuiConfig& config = s_Context->m_Config;
		const glm::vec2 labelSize = Util::CalculateTextSize(label, config.Font);
		const glm::vec2 nodeOffset = window->TreeNodeOffset();
		const glm::vec2 fullSize = glm::vec2(size.x + labelSize.x, std::max(size.y, labelSize.y)) + nodeOffset;
		if (window->NextWidgetClipped(fullSize))
		{
			window->MoveCursorPosition(fullSize);
			return 0;
		}
		InGuiID id = window->GetID(label);

		const glm::vec2 pos = window->MoveCursorPosition(fullSize) + nodeOffset;
		glm::vec4 color = config.Colors[InGuiConfig::CheckboxColor];
		InGuiRect rect(pos, pos + size);
		
		rect.Union(window->ClipRect());

		InGuiBehavior::ButtonBehavior(rect, id, result);
		if (IS_SET(result, InGui::Hoover) || frame.HighlightNext)
		{
			color = config.Colors[InGuiConfig::CheckboxHighlight];
		}
		if (IS_SET(result, InGui::Pressed))
		{
			open = !open;
		}

		size_t subTextureIndex = open ? InGuiConfig::DownArrow : InGuiConfig::RightArrow;
		window->PushQuad(
			color, config.SubTextures[subTextureIndex]->GetTexCoordsUpside(),
			pos, size
		);


		result = 0;
		InGuiRect textRect(rect.Min + glm::vec2(size.x, 0.0f), rect.Max + glm::vec2(labelSize.x, 0.0f));
		color = config.Colors[InGuiConfig::TextColor];
		InGuiBehavior::ButtonBehavior(textRect, id, result);
		if (IS_SET(result, InGui::Hoover) || frame.HighlightNext)
		{
			color = config.Colors[InGuiConfig::TextHighlight];
		}
		
		window->PushText(
			label, color, textRect.Min, textRect.Max, &labelSize
		);
		return result;
	}


	uint8_t InGui::BeginMenu(const char* label, float width)
	{		
		InGuiWindow* window = s_Context->m_FrameData.CurrentWindow;
		if (!window->IsActive || IS_SET(window->EditFlags, InGuiWindowEditFlags::Collapsed))
			return 0;
		
		InGuiFrame& frame = s_Context->m_FrameData;
		const InGuiInput& input = s_Context->m_Input;
		const InGuiConfig& config = s_Context->m_Config;
		const glm::vec2 labelSize = Util::CalculateTextSize(label, config.Font);

		InGuiID id = window->GetID(label);
		uint8_t result = 0;
		const glm::vec2 size = { width, config.MenuBarHeight };
		const glm::vec2 pos = window->FrameData.CursorPos;
		window->FrameData.CursorPos.y += size.y;
		frame.CurrentMenuWidth = width;

		glm::vec4 color = config.Colors[InGuiConfig::MenuColor];
		InGuiRect rect(pos, pos + size);

		InGuiBehavior::ButtonBehavior(rect, id, result);
		if (IS_SET(result, InGui::Hoover) || frame.HighlightNext)
		{
			color = config.Colors[InGuiConfig::MenuHighlight];
		}
		if (IS_SET(result, InGui::Pressed))
		{
			s_Context->m_MenuOpenID = (s_Context->m_MenuOpenID != id) ? id : 0;
		}
		if (s_Context->m_MenuOpenID == id)
			result |= InGui::Pressed;

		window->PushQuadOverlay(
			color, config.WhiteSubTexture->GetTexCoords(),
			pos, size, InGuiConfig::WhiteTextureIndex
		);
		window->PushTextOverlay(label, config.Colors[InGuiConfig::TextColor], rect.Min, rect.Max, nullptr);
		return result;
	}

	void InGui::EndMenu()
	{
		InGuiWindow* window = s_Context->m_FrameData.CurrentWindow;
		if (!window->IsActive || IS_SET(window->EditFlags, InGuiWindowEditFlags::Collapsed))
			return;
		
		InGuiFrame& frame = s_Context->m_FrameData;
		const InGuiConfig& config = s_Context->m_Config;
		for (const InGuiID id : frame.MenuItems)
			window->FrameData.CursorPos.y -= config.MenuItemSize.y;
		frame.MenuItems.clear();
		window->FrameData.CursorPos.x += frame.CurrentMenuWidth;
		window->FrameData.CursorPos.y -= config.MenuBarHeight;
	}

	uint8_t InGui::MenuItem(const char* label)
	{
		InGuiWindow* window = s_Context->m_FrameData.CurrentWindow;
		if (!window->IsActive || IS_SET(window->EditFlags, InGuiWindowEditFlags::Collapsed))
			return 0;

		uint8_t result = 0;
		InGuiFrame& frame = s_Context->m_FrameData;
		const InGuiInput& input = s_Context->m_Input;
		const InGuiConfig& config = s_Context->m_Config;
		const glm::vec2 labelSize = Util::CalculateTextSize(label, config.Font);

		InGuiID id = window->GetID(label);
		frame.MenuItems.push_back(id);

		const glm::vec2 pos = window->FrameData.CursorPos;
		window->FrameData.CursorPos.y += config.MenuItemSize.y;

		glm::vec4 color = config.Colors[InGuiConfig::MenuColor];
		InGuiRect rect(pos, pos + config.MenuItemSize);
		InGuiRect textRect(rect);
		rect.Union(window->ClipRect());

		InGuiBehavior::ButtonBehavior(rect, id, result);
		if (IS_SET(result, InGui::Hoover) || frame.HighlightNext)
		{
			color = config.Colors[InGuiConfig::ButtonHighlight];
		}
		if (IS_SET(result, InGui::Pressed))
		{
			s_Context->m_MenuOpenID = 0;
		}
		window->PushQuadNotClippedOverlay(
			color, config.WhiteSubTexture->GetTexCoords(),
			pos, config.MenuItemSize, InGuiConfig::WhiteTextureIndex
		);
		window->PushTextNotClippedOverlay(label, config.Colors[InGuiConfig::TextColor], textRect.Min, textRect.Max, nullptr);
		return result;
	}

	uint8_t InGui::Button(const char* label, const glm::vec2& size)
	{
		InGuiWindow* window = s_Context->m_FrameData.CurrentWindow;
		if (!window->IsActive || IS_SET(window->EditFlags, InGuiWindowEditFlags::Collapsed))
			return 0;

		uint8_t result = 0;
		const InGuiInput& input = s_Context->m_Input;
		const InGuiFrame& frame = s_Context->m_FrameData;
		const InGuiConfig& config = s_Context->m_Config;
		const glm::vec2 labelSize = Util::CalculateTextSize(label, config.Font);
		if (window->NextWidgetClipped(size))
		{
			window->MoveCursorPosition(size);
			return 0;
		}
		InGuiID id = window->GetID(label);

		const glm::vec2 pos = window->MoveCursorPosition(size);
		glm::vec4 color = config.Colors[InGuiConfig::ButtonColor];
		InGuiRect rect(pos, pos + size);
		InGuiRect textRect(rect);

		rect.Union(window->ClipRect());

		InGuiBehavior::ButtonBehavior(rect, id, result);
		if (IS_SET(result, InGui::Hoover) || frame.HighlightNext)
		{
			color = config.Colors[InGuiConfig::ButtonHighlight];
		}
		window->PushQuad(
			color, config.WhiteSubTexture->GetTexCoords(),
			pos, size, InGuiConfig::WhiteTextureIndex
		);
		window->PushTextClipped(label, config.Colors[InGuiConfig::TextColor], textRect.Min, textRect.Max, nullptr);
		return result;
	}
	uint8_t InGui::Checkbox(const char* label, const glm::vec2& size, bool& checked)
	{
		InGuiWindow* window = s_Context->m_FrameData.CurrentWindow;
		if (!window->IsActive || IS_SET(window->EditFlags, InGuiWindowEditFlags::Collapsed))
			return 0;

		uint8_t result = 0;
		const InGuiInput& input = s_Context->m_Input;
		const InGuiFrame& frame = s_Context->m_FrameData;
		const InGuiConfig& config = s_Context->m_Config;
		const glm::vec2 labelSize = Util::CalculateTextSize(label, config.Font);
		const glm::vec2 fullSize = glm::vec2(size.x + labelSize.x, std::max(size.y, labelSize.y));
		if (window->NextWidgetClipped(fullSize))
		{
			window->MoveCursorPosition(fullSize);
			return 0;
		}
		InGuiID id = window->GetID(label);

		const glm::vec2 pos = window->MoveCursorPosition(fullSize);
		glm::vec4 color = config.Colors[InGuiConfig::CheckboxColor];
		InGuiRect rect(pos, pos + size);
		InGuiRect textRect(rect);
		rect.Union(window->ClipRect());

		InGuiBehavior::ButtonBehavior(rect, id, result);
		if (IS_SET(result, InGui::Hoover))
		{
			color = config.Colors[InGuiConfig::CheckboxHighlight];
		}
		if (IS_SET(result, InGui::Pressed))
		{
			checked = !checked;
		}
		
		size_t subTextureIndex = checked ? InGuiConfig::CheckboxChecked : InGuiConfig::CheckboxUnChecked;
		window->PushQuad(
			color, config.SubTextures[subTextureIndex]->GetTexCoordsUpside(),
			pos, size
		);
		
		window->PushText(
			label, config.Colors[InGuiConfig::TextColor],
			textRect.Min + glm::vec2(size.x, 0.0f), 
			textRect.Max + glm::vec2(labelSize.x, 0.0f), 
			&labelSize
		);
		return result;
	}

	uint8_t InGui::SliderFloat(const char* label, const glm::vec2& size, float& value, float min, float max, const char* format)
	{
		InGuiWindow* window = s_Context->m_FrameData.CurrentWindow;
		if (!window->IsActive || IS_SET(window->EditFlags, InGuiWindowEditFlags::Collapsed))
			return 0;

		uint8_t result = 0;
		const InGuiInput& input = s_Context->m_Input;
		const InGuiFrame& frame = s_Context->m_FrameData;
		const InGuiConfig& config = s_Context->m_Config;
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

		glm::vec4 color = config.Colors[InGuiConfig::SliderColor];
		glm::vec4 handleColor = config.Colors[InGuiConfig::SliderHandleColor];
		InGuiRect rect(pos, pos + size);
		InGuiRect textRect(rect);
		rect.Union(window->ClipRect());

		InGuiRect handleRect(handlePos, handlePos + handleSize);
		rect.Union(window->ClipRect());

		InGuiBehavior::SliderBehavior(rect, handleRect, id, result);
		if (IS_SET(result, InGui::Hoover) || frame.HighlightNext)
		{
			color = config.Colors[InGuiConfig::SliderHighlight];
			handleColor = config.Colors[InGuiConfig::SliderHandleHighlight];
		}
		if (IS_SET(result, InGui::Pressed))
		{
			float multiplier = (input.MousePosition.x - pos.x) / size.x;
			value = std::clamp(diff * multiplier, min, max);
		}

		window->PushQuad(
			color, config.WhiteSubTexture->GetTexCoords(),
			pos, size, InGuiConfig::WhiteTextureIndex
		);
		window->PushQuad(
			handleColor, config.WhiteSubTexture->GetTexCoords(),
			handlePos, handleSize, InGuiConfig::WhiteTextureIndex
		);
		window->PushText(
			label, config.Colors[InGuiConfig::TextColor],
			textRect.Min + glm::vec2(size.x, 0.0f),
			textRect.Max + glm::vec2(labelSize.x, 0.0f),
			&labelSize
		);
		if (format)
		{
			char buffer[InGuiContext::sc_SliderValueBufferSize];
			sprintf(buffer, format, value);
			window->PushText(buffer, config.Colors[InGuiConfig::TextColor], textRect.Min, textRect.Max, nullptr);
		}
		return result;
	}

	uint8_t InGui::VSliderFloat(const char* label, const glm::vec2& size, float& value, float min, float max, const char* format)
	{
		InGuiWindow* window = s_Context->m_FrameData.CurrentWindow;
		if (!window->IsActive || IS_SET(window->EditFlags, InGuiWindowEditFlags::Collapsed))
			return 0;

		uint8_t result = 0;
		const InGuiInput& input = s_Context->m_Input;
		const InGuiFrame& frame = s_Context->m_FrameData;
		const InGuiConfig& config = s_Context->m_Config;
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

		glm::vec4 color = config.Colors[InGuiConfig::SliderColor];
		glm::vec4 handleColor = config.Colors[InGuiConfig::SliderHandleColor];
		InGuiRect rect(pos, pos + size);
		InGuiRect textRect(rect);
		rect.Union(window->ClipRect());

		InGuiRect handleRect(handlePos, handlePos + handleSize);
		rect.Union(window->ClipRect());

		InGuiBehavior::SliderBehavior(rect, handleRect, id, result);
		if (IS_SET(result, InGui::Hoover) || frame.HighlightNext)
		{
			color = config.Colors[InGuiConfig::SliderHighlight];
			handleColor = config.Colors[InGuiConfig::SliderHandleHighlight];
		}
		if (IS_SET(result, InGui::Pressed))
		{
			float multiplier = (input.MousePosition.y - pos.y) / size.y;
			value = std::clamp(diff * multiplier, min, max);
		}

		window->PushQuad(
			color, config.WhiteSubTexture->GetTexCoords(),
			pos, size, InGuiConfig::WhiteTextureIndex
		);
		window->PushQuad(
			handleColor, config.WhiteSubTexture->GetTexCoords(),
			handlePos, handleSize, InGuiConfig::WhiteTextureIndex
		);
		window->PushText(
			label, config.Colors[InGuiConfig::TextColor],
			textRect.Min + glm::vec2(size.x, 0.0f),
			textRect.Max + glm::vec2(labelSize.x, 0.0f),
			&labelSize
		);
		if (format)
		{
			char buffer[InGuiContext::sc_SliderValueBufferSize];
			sprintf(buffer, format, value);
			window->PushText(buffer, config.Colors[InGuiConfig::TextColor], textRect.Min, textRect.Max, nullptr);
		}
		return result;
	}

	uint8_t InGui::Float(const char* label, const glm::vec2& size, float& value, int32_t decimalPrecision)
	{
		InGuiWindow* window = s_Context->m_FrameData.CurrentWindow;
		if (!window->IsActive || IS_SET(window->EditFlags, InGuiWindowEditFlags::Collapsed))
			return 0;

		uint8_t result = 0;
		const InGuiInput& input = s_Context->m_Input;
		const InGuiFrame& frame = s_Context->m_FrameData;
		const InGuiConfig& config = s_Context->m_Config;
		const glm::vec2 labelSize = Util::CalculateTextSize(label, config.Font);
		const glm::vec2 fullSize = glm::vec2(size.x + labelSize.x, std::max(size.y, labelSize.y));
		if (window->NextWidgetClipped(fullSize))
		{
			window->MoveCursorPosition(fullSize);
			return 0;
		}
		InGuiID id = window->GetID(label);

		const glm::vec2 pos = window->MoveCursorPosition(fullSize);
		glm::vec4 color = config.Colors[InGuiConfig::InputColor];
		InGuiRect rect(pos, pos + size);
		InGuiRect textRect(rect);
		rect.Union(window->ClipRect());

		InGuiBehavior::FloatBehavior(rect, id, result, value, decimalPrecision);		
		if (IS_SET(result, InGui::Hoover | InGui::Pressed) || s_Context->m_LastInputID == id || frame.HighlightNext)
		{
			color = config.Colors[InGuiConfig::InputHighlight];
		}

		window->PushQuad(
			color, config.WhiteSubTexture->GetTexCoords(),
			pos, size, InGuiConfig::WhiteTextureIndex
		);
		window->PushText(
			label, config.Colors[InGuiConfig::TextColor],
			textRect.Min + glm::vec2(size.x, 0.0f),
			textRect.Max + glm::vec2(labelSize.x, 0.0f),
			&labelSize
		);
			
		if (s_Context->m_LastInputID == id)
		{ 
			window->PushTextClipped(s_Context->m_TemporaryTextBuffer.c_str(), config.Colors[InGuiConfig::TextColor], textRect.Min, textRect.Max, nullptr);
		}
		else
		{
			char buffer[InGuiContext::sc_InputValueBufferSize];
			Util::FormatFloat(buffer, InGuiContext::sc_InputValueBufferSize, value, decimalPrecision);
			window->PushTextClipped(buffer, config.Colors[InGuiConfig::TextColor], textRect.Min, textRect.Max, nullptr);
		}		
		return result;
	}

	uint8_t InGui::Float2(const char* label1, const char*label2, const glm::vec2& size, float* values, int32_t decimalPrecision)
	{
		InGui::BeginGroup();
		InGuiWindow* window = s_Context->m_FrameData.CurrentWindow;
		uint8_t result = 
			  Float(label1, size, values[0], decimalPrecision) 
			| Float(label2, size, values[1], decimalPrecision);
		InGui::EndGroup();
		return result;
	}

	uint8_t InGui::Float3(const char* label1, const char* label2, const char* label3, const glm::vec2& size, float* values, int32_t decimalPrecision)
	{
		InGui::BeginGroup();
		InGuiWindow* window = s_Context->m_FrameData.CurrentWindow;
		uint8_t result = 
			  Float(label1, size, values[0], decimalPrecision) 
			| Float(label2, size, values[1], decimalPrecision)
			| Float(label3, size, values[2], decimalPrecision);

		InGui::EndGroup();
		return result;
	}

	uint8_t InGui::Float4(const char* label1, const char* label2, const char* label3, const char* label4, const glm::vec2& size, float* values, int32_t decimalPrecision)
	{
		InGui::BeginGroup();
		InGuiWindow* window = s_Context->m_FrameData.CurrentWindow;
		uint8_t result =
			  Float(label1, size, values[0], decimalPrecision)
			| Float(label2, size, values[1], decimalPrecision)
			| Float(label3, size, values[2], decimalPrecision)
			| Float(label4, size, values[3], decimalPrecision);
		InGui::EndGroup();
		return result;
	}

	uint8_t InGui::Int(const char* label, const glm::vec2& size, int32_t& value)
	{
		InGuiWindow* window = s_Context->m_FrameData.CurrentWindow;
		if (!window->IsActive || IS_SET(window->EditFlags, InGuiWindowEditFlags::Collapsed))
			return 0;

		uint8_t result = 0;
		const InGuiInput& input = s_Context->m_Input;
		const InGuiFrame& frame = s_Context->m_FrameData;
		const InGuiConfig& config = s_Context->m_Config;
		const glm::vec2 labelSize = Util::CalculateTextSize(label, config.Font);
		const glm::vec2 fullSize = glm::vec2(size.x + labelSize.x, std::max(size.y, labelSize.y));
		if (window->NextWidgetClipped(fullSize))
		{
			window->MoveCursorPosition(fullSize);
			return 0;
		}
		InGuiID id = window->GetID(label);

		const glm::vec2 pos = window->MoveCursorPosition(fullSize);
		glm::vec4 color = config.Colors[InGuiConfig::InputColor];
		InGuiRect rect(pos, pos + size);
		InGuiRect textRect(rect);
		rect.Union(window->ClipRect());

		InGuiBehavior::IntBehavior(rect, id, result, value);
		if (IS_SET(result, InGui::Hoover | InGui::Pressed) || s_Context->m_LastInputID == id || frame.HighlightNext)
		{
			color = config.Colors[InGuiConfig::InputHighlight];
		}

		window->PushQuad(
			color, config.WhiteSubTexture->GetTexCoords(),
			pos, size, InGuiConfig::WhiteTextureIndex
		);
		window->PushText(
			label, config.Colors[InGuiConfig::TextColor],
			textRect.Min + glm::vec2(size.x, 0.0f),
			textRect.Max + glm::vec2(labelSize.x, 0.0f),
			&labelSize
		);

		if (s_Context->m_LastInputID == id)
		{
			window->PushTextClipped(s_Context->m_TemporaryTextBuffer.c_str(), config.Colors[InGuiConfig::TextColor], textRect.Min, textRect.Max, nullptr);
		}
		else
		{
			char buffer[InGuiContext::sc_InputValueBufferSize];
			sprintf(buffer, "%d", value);
			window->PushTextClipped(buffer, config.Colors[InGuiConfig::TextColor], textRect.Min, textRect.Max, nullptr);
		}
		return result;
	}

	uint8_t InGui::UInt(const char* label, const glm::vec2& size, uint32_t& value)
	{
		InGuiWindow* window = s_Context->m_FrameData.CurrentWindow;
		if (!window->IsActive || IS_SET(window->EditFlags, InGuiWindowEditFlags::Collapsed))
			return 0;

		uint8_t result = 0;
		const InGuiInput& input = s_Context->m_Input;
		const InGuiFrame& frame = s_Context->m_FrameData;
		const InGuiConfig& config = s_Context->m_Config;
		const glm::vec2 labelSize = Util::CalculateTextSize(label, config.Font);
		const glm::vec2 fullSize = glm::vec2(size.x + labelSize.x, std::max(size.y, labelSize.y));
		if (window->NextWidgetClipped(fullSize))
		{
			window->MoveCursorPosition(fullSize);
			return 0;
		}
		InGuiID id = window->GetID(label);

		const glm::vec2 pos = window->MoveCursorPosition(fullSize);
		glm::vec4 color = config.Colors[InGuiConfig::InputColor];
		InGuiRect rect(pos, pos + size);
		InGuiRect textRect(rect);
		rect.Union(window->ClipRect());

		InGuiBehavior::UIntBehavior(rect, id, result, value);
		if (IS_SET(result, InGui::Hoover | InGui::Pressed) || s_Context->m_LastInputID == id || frame.HighlightNext)
		{
			color = config.Colors[InGuiConfig::InputHighlight];
		}

		window->PushQuad(
			color, config.WhiteSubTexture->GetTexCoords(),
			pos, size, InGuiConfig::WhiteTextureIndex
		);
		window->PushText(
			label, config.Colors[InGuiConfig::TextColor],
			textRect.Min + glm::vec2(size.x, 0.0f),
			textRect.Max + glm::vec2(labelSize.x, 0.0f),
			&labelSize
		);

		if (s_Context->m_LastInputID == id)
		{
			window->PushTextClipped(s_Context->m_TemporaryTextBuffer.c_str(), config.Colors[InGuiConfig::TextColor], textRect.Min, textRect.Max, nullptr);
		}
		else
		{
			char buffer[InGuiContext::sc_InputValueBufferSize];
			sprintf(buffer, "%u", value);
			window->PushTextClipped(buffer, config.Colors[InGuiConfig::TextColor], textRect.Min, textRect.Max, nullptr);
		}
		return result;
	}

	uint8_t InGui::String(const char* label, const glm::vec2& size, std::string& value)
	{
		InGuiWindow* window = s_Context->m_FrameData.CurrentWindow;
		if (!window->IsActive || IS_SET(window->EditFlags, InGuiWindowEditFlags::Collapsed))
			return 0;

		uint8_t result = 0;
		const InGuiInput& input = s_Context->m_Input;
		const InGuiFrame& frame = s_Context->m_FrameData;
		const InGuiConfig& config = s_Context->m_Config;
		const glm::vec2 labelSize = Util::CalculateTextSize(label, config.Font);
		const glm::vec2 fullSize = glm::vec2(size.x + labelSize.x, std::max(size.y, labelSize.y));
		if (window->NextWidgetClipped(fullSize))
		{
			window->MoveCursorPosition(fullSize);
			return 0;
		}
		InGuiID id = window->GetID(label);

		const glm::vec2 pos = window->MoveCursorPosition(fullSize);
		glm::vec4 color = config.Colors[InGuiConfig::InputColor];
		InGuiRect rect(pos, pos + size);
		InGuiRect textRect(rect);
		rect.Union(window->ClipRect());

		InGuiBehavior::StringBehavior(rect, id, result, value);
		if (IS_SET(result, InGui::Hoover | InGui::Pressed) || s_Context->m_LastInputID == id || frame.HighlightNext)
		{
			color = config.Colors[InGuiConfig::InputHighlight];
		}

		window->PushQuad(
			color, config.WhiteSubTexture->GetTexCoords(),
			pos, size, InGuiConfig::WhiteTextureIndex
		);
		window->PushText(
			label, config.Colors[InGuiConfig::TextColor],
			textRect.Min + glm::vec2(size.x, 0.0f),
			textRect.Max + glm::vec2(labelSize.x, 0.0f),
			&labelSize
		);

		if (s_Context->m_LastInputID == id)
		{
			window->PushTextClipped(s_Context->m_TemporaryTextBuffer.c_str(), config.Colors[InGuiConfig::TextColor], textRect.Min, textRect.Max, nullptr);
		}
		else
		{
			window->PushTextClipped(value.c_str(), config.Colors[InGuiConfig::TextColor], textRect.Min, textRect.Max, nullptr);
		}
		return result;
	}
	uint8_t InGui::Image(const char* label, const glm::vec2& size, const Ref<SubTexture>& subTexture)
	{
		InGuiWindow* window = s_Context->m_FrameData.CurrentWindow;
		if (!window->IsActive || IS_SET(window->EditFlags, InGuiWindowEditFlags::Collapsed))
			return 0;

		uint8_t result = 0;
		InGuiFrame& frame = s_Context->m_FrameData;
		const InGuiInput& input = s_Context->m_Input;
		const InGuiConfig& config = s_Context->m_Config;
		const glm::vec2 labelSize = Util::CalculateTextSize(label, config.Font);
		const glm::vec2 fullSize = glm::vec2(size.x + labelSize.x, std::max(size.y, labelSize.y));
		if (window->NextWidgetClipped(fullSize))
		{
			window->MoveCursorPosition(fullSize);
			return 0;
		}
		InGuiID id = window->GetID(label);

		const glm::vec2 pos = window->MoveCursorPosition(fullSize);
		glm::vec4 color = config.Colors[InGuiConfig::ImageColor];
		InGuiRect rect(pos, pos + size);
		InGuiRect textRect(rect);
		rect.Union(window->ClipRect());

		InGuiBehavior::ButtonBehavior(rect, id, result);
		if (IS_SET(result, InGui::Hoover) || frame.HighlightNext)
		{
			color = config.Colors[InGuiConfig::ImageHighlight];
		}

		window->PushQuad(
			color, subTexture->GetTexCoords(),
			pos, size, frame.AddCustomTexture(subTexture->GetTexture())
		);
		window->PushText(
			label, config.Colors[InGuiConfig::TextColor],
			textRect.Min + glm::vec2(size.x, 0.0f),
			textRect.Max + glm::vec2(labelSize.x, 0.0f),
			&labelSize
		);
		return result;
	}

	InGuiContext& InGui::GetContext()
	{
		return *s_Context;
	}
}