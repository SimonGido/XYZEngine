#include "stdafx.h"
#include "InGuiWindow.h"

#include "InGuiUtil.h"
#include "InGui.h"

namespace XYZ {	
	InGuiWindow::InGuiWindow(uint32_t workClipId, uint32_t panelClipID)
		:
		ClipID(workClipId),
		WorkClipID(workClipId),
		PanelClipID(panelClipID),
		EditFlags(0),
		StyleFlags(0),
		Position(100.0f),
		Size(300.0f),
		OriginalSize(Size),
		Scroll(0.0f),
		IsActive(true),
		IsInitialized(false),
		FrameData(this),
		Axis(AxisPlacement::Vertical),
		TabID(InGuiInvalidID),
		ScrollBarX(false),
		ScrollBarY(false),
		Grouping(false),
		ScrollBarSize(10.0f),
		DrawListInUse(nullptr),
		Parent(nullptr),
		DockNode(nullptr)
	{
		StyleFlags |= InGuiWindowStyleFlags::PanelEnabled;
		StyleFlags |= InGuiWindowStyleFlags::ScrollEnabled;
	}

	void InGuiWindow::PushItselfToDrawlist(bool highlight, InGuiClipID clipID)
	{
		const InGuiConfig& config = InGui::GetContext().m_Config;
		size_t subTextureIndex = IS_SET(EditFlags, InGuiWindowEditFlags::Collapsed) ? InGuiConfig::LeftArrow : InGuiConfig::DownArrow;
		const Ref<SubTexture>& minimizeSubTexture = config.SubTextures[subTextureIndex];
		
		if (!IS_SET(EditFlags, InGuiWindowEditFlags::Collapsed))
		{
			if (IS_SET(StyleFlags, InGuiWindowStyleFlags::FrameEnabled))
			{
				InGuiRect rect = RealRect();
				rect += config.WindowFrameThickness;
				DrawListInUse->PushQuad(
					config.Colors[InGuiConfig::WindowFrameColor], 
					config.WhiteSubTexture->GetTexCoords(),
					rect.Min, rect.Max - rect.Min, 
					InGuiConfig::WhiteTextureIndex, clipID
				);
			}
			DrawListInUse->PushQuad(
				config.Colors[InGuiConfig::WindowColor], 
				config.WhiteSubTexture->GetTexCoords(),
				Position, Size, InGuiConfig::WhiteTextureIndex, clipID
			);
		}
		if (IS_SET(StyleFlags, InGuiWindowStyleFlags::PanelEnabled))
		{
			glm::vec4 panelColor = highlight ? config.Colors[InGuiConfig::WindowPanelHighlight] : config.Colors[InGuiConfig::WindowPanelColor];
			DrawListInUse->PushQuad(
				panelColor, config.WhiteSubTexture->GetTexCoords(),
				Position, { Size.x, config.PanelHeight }, 
				InGuiConfig::WhiteTextureIndex, clipID
			);

			glm::vec2 buttonPosition = Position + glm::vec2(Size.x - config.PanelHeight, 0.0f);
			DrawListInUse->PushQuad(
				config.Colors[InGuiConfig::ButtonColor], 
				minimizeSubTexture->GetTexCoordsUpside(),
				buttonPosition, { config.PanelHeight, config.PanelHeight }, 
				InGuiConfig::DefaultTextureIndex, clipID
			);
			if (IS_SET(StyleFlags, InGuiWindowStyleFlags::LabelEnabled))
			{
				glm::vec2 textPosition = glm::vec2(Position.x + config.LabelOffset, Position.y + config.PanelHeight);
				textPosition.y -= std::floor((config.PanelHeight - config.Font->GetLineHeight()) / 2.0f);
				Util::GenerateTextMesh(
					Name.c_str(), config.Font, config.Colors[InGuiConfig::TextColor],
					textPosition, DrawListInUse->m_Quads,
					InGuiConfig::FontTextureIndex, PanelClipID
				);
			}
		}
	}

	void InGuiWindow::PushTextClipped(const char* text, const glm::vec4& color, const glm::vec2& posMin, const glm::vec2& posMax, const glm::vec2* textSize)
	{
		InGuiContext& context = InGui::GetContext();
		const Ref<Font>& font = context.m_Config.Font;
		glm::vec2 size = posMax - posMin;
		glm::vec2 textPosition = posMin;
		glm::vec2 tmpTextSize(0.0f);
		if (textSize)
			tmpTextSize = *textSize;
		else
		{
			uint32_t numCharacters = Util::CalculateNumCharacters(text, font, size);
			tmpTextSize = Util::CalculateTextSize(text, font, numCharacters);
		}
		textPosition.x += (size.x - tmpTextSize.x) / 2.0f;
		textPosition.y += std::floor((size.y - tmpTextSize.y) / 2.0f) + font->GetLineHeight();
		Util::GenerateTextMeshClipped(
			text, font, color, textPosition,
			DrawListInUse->m_Quads, InGuiConfig::FontTextureIndex,
			ClipID, size
		);	
	}

	void InGuiWindow::PushTextNotClipped(const char* text, const glm::vec4& color, const glm::vec2& posMin, const glm::vec2& posMax, const glm::vec2* textSize)
	{
		InGuiContext& context = InGui::GetContext();
		const Ref<Font>& font = context.m_Config.Font;
		glm::vec2 size = posMax - posMin;
		glm::vec2 textPosition = posMin;
		glm::vec2 tmpTextSize(0.0f);
		if (textSize)
			tmpTextSize = *textSize;
		else
			tmpTextSize = Util::CalculateTextSize(text, font);

		textPosition.x += (size.x - tmpTextSize.x) / 2.0f;
		textPosition.y += std::floor((size.y - tmpTextSize.y) / 2.0f) + font->GetLineHeight();
		Util::GenerateTextMesh(
			text, font, color, textPosition,
			DrawListInUse->m_Quads, InGuiConfig::FontTextureIndex,
			0
		);
	}

	void InGuiWindow::PushText(const char* text, const glm::vec4& color, const glm::vec2& posMin, const glm::vec2& posMax, const glm::vec2* textSize)
	{
		InGuiContext& context = InGui::GetContext();
		const Ref<Font>& font = context.m_Config.Font;
		glm::vec2 size = posMax - posMin;
		glm::vec2 textPosition = posMin;
		glm::vec2 tmpTextSize(0.0f);
		if (textSize)
			tmpTextSize = *textSize;
		else
			tmpTextSize = Util::CalculateTextSize(text, font);

		textPosition.x += (size.x - tmpTextSize.x) / 2.0f;
		textPosition.y += std::floor((size.y - tmpTextSize.y) / 2.0f) + font->GetLineHeight();
		Util::GenerateTextMesh(
			text, font, color, textPosition,
			DrawListInUse->m_Quads, InGuiConfig::FontTextureIndex,
			ClipID
		);
	}

	void InGuiWindow::PushQuad(const glm::vec4& color, const glm::vec4& texCoord, const glm::vec2& pos, const glm::vec2& size, uint32_t textureID)
	{
		DrawListInUse->PushQuad(
			color, texCoord,
			pos, size, 
			textureID, ClipID
		);
	}

	void InGuiWindow::PushQuadNotClipped(const glm::vec4& color, const glm::vec4& texCoord, const glm::vec2& pos, const glm::vec2& size, uint32_t textureID)
	{
		DrawListInUse->PushQuad(
			color, texCoord,
			pos, size,
			textureID, 0
		);
	}

	void InGuiWindow::PushLine(const glm::vec2& p0, const glm::vec2& p1, const glm::vec4& color)
	{
		DrawListInUse->PushLine(color, p0, p1, ClipID);
	}

	void InGuiWindow::PushTextClippedOverlay(const char* text, const glm::vec4& color, const glm::vec2& posMin, const glm::vec2& posMax, const glm::vec2* textSize)
	{
		InGuiContext& context = InGui::GetContext();
		const Ref<Font>& font = context.m_Config.Font;
		glm::vec2 size = posMax - posMin;
		glm::vec2 textPosition = posMin;
		glm::vec2 tmpTextSize(0.0f);
		if (textSize)
			tmpTextSize = *textSize;
		else
		{
			uint32_t numCharacters = Util::CalculateNumCharacters(text, font, size);
			tmpTextSize = Util::CalculateTextSize(text, font, numCharacters);
		}
		textPosition.x += (size.x - tmpTextSize.x) / 2.0f;
		textPosition.y += std::floor((size.y - tmpTextSize.y) / 2.0f) + font->GetLineHeight();
		Util::GenerateTextMeshClipped(
			text, font, color, textPosition,
			DrawListInUse->m_QuadsOverlay, InGuiConfig::FontTextureIndex,
			ClipID, size
		);
	}

	void InGuiWindow::PushTextNotClippedOverlay(const char* text, const glm::vec4& color, const glm::vec2& posMin, const glm::vec2& posMax, const glm::vec2* textSize)
	{
		InGuiContext& context = InGui::GetContext();
		const Ref<Font>& font = context.m_Config.Font;
		glm::vec2 size = posMax - posMin;
		glm::vec2 textPosition = posMin;
		glm::vec2 tmpTextSize(0.0f);
		if (textSize)
			tmpTextSize = *textSize;
		else
			tmpTextSize = Util::CalculateTextSize(text, font);

		textPosition.x += (size.x - tmpTextSize.x) / 2.0f;
		textPosition.y += std::floor((size.y - tmpTextSize.y) / 2.0f) + font->GetLineHeight();
		Util::GenerateTextMesh(
			text, font, color, textPosition,
			DrawListInUse->m_QuadsOverlay, InGuiConfig::FontTextureIndex,
			0
		);
	}

	void InGuiWindow::PushTextOverlay(const char* text, const glm::vec4& color, const glm::vec2& posMin, const glm::vec2& posMax, const glm::vec2* textSize)
	{
		InGuiContext& context = InGui::GetContext();
		const Ref<Font>& font = context.m_Config.Font;
		glm::vec2 size = posMax - posMin;
		glm::vec2 textPosition = posMin;
		glm::vec2 tmpTextSize(0.0f);
		if (textSize)
			tmpTextSize = *textSize;
		else
			tmpTextSize = Util::CalculateTextSize(text, font);

		textPosition.x += (size.x - tmpTextSize.x) / 2.0f;
		textPosition.y += std::floor((size.y - tmpTextSize.y) / 2.0f) + font->GetLineHeight();
		Util::GenerateTextMesh(
			text, font, color, textPosition,
			DrawListInUse->m_QuadsOverlay, InGuiConfig::FontTextureIndex,
			ClipID
		);
	}

	void InGuiWindow::PushQuadOverlay(const glm::vec4& color, const glm::vec4& texCoord, const glm::vec2& pos, const glm::vec2& size, uint32_t textureID)
	{
		DrawListInUse->PushQuadOverlay(
			color, texCoord,
			pos, size,
			textureID, ClipID
		);
	}

	void InGuiWindow::PushQuadNotClippedOverlay(const glm::vec4& color, const glm::vec4& texCoord, const glm::vec2& pos, const glm::vec2& size, uint32_t textureID)
	{
		DrawListInUse->PushQuadOverlay(
			color, texCoord,
			pos, size,
			textureID, 0
		);
	}

	void InGuiWindow::SetParent(InGuiWindow* parent)
	{
		if (parent)
		{
			DrawListInUse = parent->DrawListInUse;
			Position = parent->MoveCursorPosition(GetRealSize());
			parent->ChildWindows.push_back(this);
		}
		else
		{
			if (Parent)
			{
				auto it = std::find(Parent->ChildWindows.begin(), Parent->ChildWindows.end(), this);
				Parent->ChildWindows.erase(it);
			}
			DrawListInUse = &DrawList;
		}
		Parent = parent;
	}

	bool InGuiWindow::NextWidgetClipped(const glm::vec2& size)
	{
		InGuiRect rect(FrameData.CursorPos, FrameData.CursorPos + size);
		if (IS_SET(StyleFlags, InGuiWindowStyleFlags::ScrollEnabled))
			rect.Translate(-Scroll);
		return !(rect.Overlaps(ClipRect()));
	}

	bool InGuiWindow::NextTabClipped()
	{
		return (FrameData.TabOffet > Position.x + Size.x);
	}

	glm::vec2 InGuiWindow::MoveCursorPosition(const glm::vec2& size)
	{
		glm::vec2 old = FrameData.CursorPos;
		const glm::vec2& padding = InGui::GetContext().m_Config.WindowPadding;
		float xScrollMax = FrameData.CursorPos.x - (Position.x + Size.x) + size.x + padding.x;
		float yScrollMax = FrameData.CursorPos.y - (Position.y + Size.y) + size.y + padding.y;
		if (Axis == AxisPlacement::Horizontal)
		{
			if (Grouping)
			{
				FrameData.GroupSize.y += size.y;
				FrameData.GroupSize.x = std::max(size.x, FrameData.GroupSize.x);
				FrameData.Params.RowHeight = std::max(FrameData.Params.RowHeight, FrameData.GroupSize.y);
				FrameData.CursorPos.y += size.y + padding.y;
			}
			else
			{
				FrameData.Params.RowHeight = std::max(FrameData.Params.RowHeight, size.y);
				FrameData.CursorPos.x += size.x + padding.x;
			}
		}
		else
		{
			if (Grouping)
			{
				FrameData.GroupSize.x += size.x;
				FrameData.GroupSize.y = std::max(size.y, FrameData.GroupSize.y);
				FrameData.Params.RowWidth = std::max(FrameData.Params.RowWidth, FrameData.GroupSize.x);				
				FrameData.CursorPos.x += size.x + padding.x;
			}
			else
			{
				FrameData.Params.RowWidth = std::max(FrameData.Params.RowWidth, size.x);
				FrameData.CursorPos.y += size.y + padding.y;
			}
		}
		FrameData.ScrollMax.x = std::max(FrameData.ScrollMax.x, xScrollMax);
		FrameData.ScrollMax.y = std::max(FrameData.ScrollMax.y, yScrollMax);
		
		old -= Scroll;
		return old;
	}

	float InGuiWindow::MoveTabPosition(float size)
	{
		float old = FrameData.TabOffet;
		FrameData.TabOffet += size;
		return old;
	}

	void InGuiWindow::SetCursorPosition(const glm::vec2& position)
	{
		FrameData.CursorPos = position;
	}

	void InGuiWindow::EndGroup()
	{
		XYZ_ASSERT(Grouping, "Grouping was not enabled");
		Grouping = false;
		const glm::vec2& padding = InGui::GetContext().m_Config.WindowPadding;
		if (Axis == AxisPlacement::Horizontal)
		{
			FrameData.CursorPos.x += FrameData.GroupSize.x + padding.x;
			FrameData.CursorPos.y = Position.y + padding.y;
			FrameData.GroupSize = glm::vec2(0.0f);
		}
		else
		{
			FrameData.CursorPos.y += FrameData.GroupSize.y + padding.y;
			FrameData.CursorPos.x = Position.x + padding.x;
			FrameData.GroupSize = glm::vec2(0.0f);
		}
	}

	bool InGuiWindow::ResolveResizeFlags(const glm::vec2& mousePosition, bool checkCollision)
	{
		// Window is docked, can not be resized
		if (DockNode)
			return false;

		float threshhold = InGui::GetContext().m_Config.ResizeThreshhold;
		if (checkCollision)
		{
			if (!RealRect().Overlaps(mousePosition))
				return false;
		}

		bool result = false;
		if (mousePosition.x < Position.x + threshhold)
		{
			EditFlags |= InGuiWindowEditFlags::ResizeLeft;
			result = true;
		}
		else if (mousePosition.x > Position.x + Size.x - threshhold)
		{
			EditFlags |= InGuiWindowEditFlags::ResizeRight;
			result = true;
		}

		if (mousePosition.y > Position.y + Size.y - threshhold)
		{
			EditFlags |= InGuiWindowEditFlags::ResizeBottom;
			result = true;
		}
		return result;
	}

	bool InGuiWindow::HandleResize(const glm::vec2& mousePosition)
	{
		bool result = false;
		if (IS_SET(EditFlags, InGuiWindowEditFlags::ResizeRight))
		{
			Size.x = mousePosition.x - Position.x;
			result = true;
		}
		else if (IS_SET(EditFlags, InGuiWindowEditFlags::ResizeLeft))
		{
			float oldX = Position.x;
			Position.x = mousePosition.x;
			Size.x -= Position.x - oldX;
		}
		if (IS_SET(EditFlags, InGuiWindowEditFlags::ResizeBottom))
		{
			Size.y = mousePosition.y - Position.y;
			result = true;
		}

		return result;
	}

	bool InGuiWindow::HandleHoover(const glm::vec2& mousePosition)
	{
		InGuiRect windowRectangle = Rect();
		if ((IsFocused() || IsChild()) && windowRectangle.Overlaps(mousePosition))
		{
			EditFlags |= InGuiWindowEditFlags::Hoovered;
			InGuiRect panelRectangle = PanelRect();
			return panelRectangle.Overlaps(mousePosition);
		}
		else
		{
			EditFlags &= ~InGuiWindowEditFlags::Hoovered;
		}
		return false;
	}

	bool InGuiWindow::HandleMove(const glm::vec2& mousePosition, const glm::vec2& offset)
	{
		if (IS_SET(EditFlags, InGuiWindowEditFlags::Moving))
		{
			Position = mousePosition - offset;
			return true;
		}
		return false;
	}

	void InGuiWindow::HandleScrollbars()
	{
		ScrollBarX = FrameData.ScrollMax.x > 0.0f;
		ScrollBarY = FrameData.ScrollMax.y > 0.0f;
		if (!ScrollBarX) Scroll.x = 0.0f;
		if (!ScrollBarY) Scroll.y = 0.0f;
		
		// Backup cursor

		float threshhold = InGui::GetContext().m_Config.ResizeThreshhold;
		glm::vec2 oldCursor = FrameData.CursorPos;
		InGuiRect rect = ClipRect();
		if (ScrollBarY)
		{
			SetCursorPosition(glm::vec2(
				rect.Max.x - ScrollBarSize.y - threshhold,
				rect.Min.y + threshhold)
			);
			float yScroll = Scroll.y;
			float ySize = rect.Max.y - rect.Min.y - (2.0f * threshhold);
			Scroll.y = 0.0f;
			InGui::VSliderFloat(
				"", glm::vec2(ScrollBarSize.y, ySize),
				yScroll, 0.0f, FrameData.ScrollMax.y, nullptr
			);
			Scroll.y = yScroll;
		}
		else if (ScrollBarX)
		{
			SetCursorPosition(glm::vec2(
				rect.Min.x + threshhold,
				rect.Max.y - ScrollBarSize.y - threshhold)
			);
			float xScroll = Scroll.x;
			float xSize = rect.Max.x - rect.Min.x - (2.0f * threshhold);
			Scroll.x = 0.0f;
			InGui::SliderFloat(
				"", glm::vec2(xSize, ScrollBarSize.y),
				xScroll, 0.0f, FrameData.ScrollMax.x, nullptr
			);
			Scroll.x = xScroll;
		}
		// Restore cursor
		SetCursorPosition(oldCursor);
	}

	bool InGuiWindow::IsResizing() const
	{
		const InGuiDockSpace& dockSpace = InGui::GetContext().m_DockSpace;
		if (DockNode && dockSpace.ResizedNode)
			return true;
		return IS_SET(EditFlags,
			  InGuiWindowEditFlags::ResizeBottom
			| InGuiWindowEditFlags::ResizeLeft
			| InGuiWindowEditFlags::ResizeRight);
	}

	bool InGuiWindow::IsFocused() const
	{
		return this == InGui::GetContext().m_FocusedWindow;
	}

	bool InGuiWindow::IsChild() const
	{
		return Parent;
	}

	bool InGuiWindow::IsParentFocused() const
	{
		if (Parent)
			return Parent->IsFocused();
		return false;
	}

	InGuiID InGuiWindow::GetID(const char* name) const
	{
		size_t id = 0;
		HashCombine(id, std::string_view(Name.c_str()), std::string_view(name));
		return id;
	}

	glm::vec2 InGuiWindow::GetRealSize() const
	{
		glm::vec2 result(0.0f);
		if (IsActive)
		{
			result = { Size.x, InGui::GetContext().m_Config.PanelHeight };
		}
		if (!IS_SET(EditFlags, InGuiWindowEditFlags::Collapsed))
		{
			result.y = Size.y;
		}
		return result;
	}

	glm::vec2 InGuiWindow::TreeNodeOffset() const
	{
		const InGuiFrame& frame =   InGui::GetContext().m_FrameData;
		const InGuiConfig& config = InGui::GetContext().m_Config;
		glm::vec2 nodeOffset(0.0f);
		if (Axis == AxisPlacement::Horizontal)
			nodeOffset.y += frame.CurrentTreeDepth * config.TreeNodeOffset;
		else
			nodeOffset.x += frame.CurrentTreeDepth * config.TreeNodeOffset;

		return nodeOffset;
	}

	InGuiRect InGuiWindow::Rect() const
	{
		InGuiRect rect(Position, Position + Size);
		if (Parent)
			rect.Union(Parent->Rect());
		return rect;
	}

	InGuiRect InGuiWindow::RealRect() const
	{
		glm::vec2 realSize = GetRealSize();
		InGuiRect rect(Position, Position + realSize);
		if (Parent)
			rect.Union(Parent->RealRect());
		return rect;
	}

	InGuiRect InGuiWindow::PanelRect() const
	{
		const InGuiConfig& config = InGui::GetContext().m_Config;
		InGuiRect rect(Position, Position + glm::vec2(Size.x, config.PanelHeight));
		if (Parent)
			rect.Union(Parent->Rect());
		return rect;
	}

	InGuiRect InGuiWindow::PanelClipRect(uint32_t viewportHeight) const
	{
		const InGuiConfig& config = InGui::GetContext().m_Config;
		glm::vec2 min = { Position.x, (float)viewportHeight - Position.y - config.PanelHeight };
		glm::vec2 max = min + glm::vec2(Size.x, config.PanelHeight);
		InGuiRect rect(min, max);
		if (Parent)
			rect.Union(Parent->ClipRect(viewportHeight));
		return rect;
	}

	InGuiRect InGuiWindow::MinimizeRect() const
	{
		const InGuiConfig& config = InGui::GetContext().m_Config;
		InGuiRect rect(
			Position + glm::vec2(Size.x - config.PanelHeight, 0.0f), 
			Position + glm::vec2(Size.x,  config.PanelHeight)
		);
		if (Parent)
			rect.Union(Parent->Rect());
		return rect;
	}

	InGuiRect InGuiWindow::ClipRect() const
	{
		const InGuiConfig& config = InGui::GetContext().m_Config;
		glm::vec2 min = { Position.x, Position.y + config.PanelHeight };
		InGuiRect rect(min, min + glm::vec2(Size.x, Size.y - config.PanelHeight));
		if (IS_SET(StyleFlags, InGuiWindowStyleFlags::MenuEnabled))
			rect.Min.y += config.MenuBarHeight;

		if (Parent)
			rect.Union(Parent->ClipRect());
		return rect;
	}

	InGuiRect InGuiWindow::ClipRect(uint32_t viewportHeight) const
	{
		const InGuiConfig& config = InGui::GetContext().m_Config;
		glm::vec2 min = { Position.x, (float)viewportHeight - Position.y - Size.y };
		glm::vec2 max = min + glm::vec2(Size.x, Size.y - config.PanelHeight);	
		InGuiRect rect(min, max);
		if (Parent)
			rect.Union(Parent->ClipRect(viewportHeight));
		return rect;
	}	
	InGuiWindowFrameData::InGuiWindowFrameData(InGuiWindow* window)
		:
		CursorPos(window->Position.x, window->Position.y + InGui::GetContext().m_Config.PanelHeight),
		ScrollMax(0.0f),
		GroupSize(0.0f)
	{
		const InGuiConfig& config = InGui::GetContext().m_Config;
		Params.RowWidth = 0.0f;
		CursorPos += config.WindowPadding;
		if (IS_SET(window->StyleFlags, InGuiWindowStyleFlags::MenuEnabled))
			CursorPos.y += config.MenuBarHeight;

		TabOffet = window->Position.x;
		if (IS_SET(window->StyleFlags, InGuiWindowStyleFlags::LabelEnabled))
		{
			TabOffet += Util::CalculateTextSize(window->Name.c_str(), InGui::GetContext().m_Config.Font).x + config.LabelOffset;
		}
	}
}