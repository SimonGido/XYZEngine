#include "stdafx.h"
#include "BasicUITypes.h"

#include "BasicUIRenderer.h"
#include "BasicUI.h"

namespace XYZ {
	namespace Helper {
		static bool Collide(const glm::vec2& pos, const glm::vec2& size, const glm::vec2& point)
		{
			return (pos.x + size.x > point.x &&
				pos.x		   < point.x&&
				pos.y + size.y >  point.y &&
				pos.y < point.y);
		}
	}
	bUIElement::bUIElement(const glm::vec2& coords, const glm::vec2& size, const glm::vec4& color, const std::string& label, const std::string& name, bUIElementType type)
		:
		Coords(coords),
		Size(size),
		Color(color),
		ActiveColor(color),
		Label(label),
		Name(name),
		Parent(nullptr),
		Visible(true),
		ChildrenVisible(true),
		Type(type)
	{
	}
	bool bUIElement::OnMouseMoved(const glm::vec2& mousePosition)
	{
		if (Helper::Collide(GetAbsolutePosition(), Size, mousePosition))
		{
			ActiveColor = bUI::GetConfig().GetColor(bUIConfig::HighlightColor);
			for (auto& callback : Callbacks)
				callback(bUICallbackType::Hoover, *this);
			return true;
		}
		ActiveColor = Color;
		return false;
	}
	bool bUIElement::OnLeftMousePressed(const glm::vec2& mousePosition)
	{
		if (Helper::Collide(GetAbsolutePosition(), Size, mousePosition))
		{
			for (auto& callback : Callbacks)
				callback(bUICallbackType::Active, *this);
			return true;
		}
		return false;
	}
	bool bUIElement::OnRightMousePressed(const glm::vec2& mousePosition)
	{
		return Helper::Collide(GetAbsolutePosition(), Size, mousePosition);
	}
	glm::vec2 bUIElement::GetAbsolutePosition() const
	{
		if (Parent)
		{
			return Parent->GetAbsolutePosition() + Coords;
		}
		return Coords;
	}
	uint32_t bUIElement::depth()
	{
		if (Parent)
		{
			return Parent->depth() + 1;
		}
		return 0;
	}
	bUIButton::bUIButton(const glm::vec2& coords, const glm::vec2& size, const glm::vec4& color, const std::string& label, const std::string& name, bUIElementType type)
		:
		bUIElement(coords, size, color,  label, name, type)
	{
	}
	void bUIButton::PushQuads(bUIRenderer& renderer, uint32_t& scissorID)
	{
		renderer.Submit<bUIButton>(*this, scissorID, bUI::GetContext().Config.GetSubTexture(bUIConfig::Button));
	}
	
	bUICheckbox::bUICheckbox(const glm::vec2& coords, const glm::vec2& size, const glm::vec4& color,  const std::string& label, const std::string& name, bUIElementType type)
		:
		bUIElement(coords, size, color,  label, name, type),
		Checked(false)
	{
	}
	void bUICheckbox::PushQuads(bUIRenderer& renderer, uint32_t& scissorID)
	{
		if (Checked)
			renderer.Submit<bUICheckbox>(*this, scissorID, bUI::GetContext().Config.GetSubTexture(bUIConfig::CheckboxChecked));
		else
			renderer.Submit<bUICheckbox>(*this, scissorID, bUI::GetContext().Config.GetSubTexture(bUIConfig::CheckboxUnChecked));
	}

	void bUICheckbox::OnUpdate()
	{
		if (Checked)
		{
			for (auto& callback : Callbacks)
				callback(bUICallbackType::Active, *this);
		}
	}

	bool bUICheckbox::OnLeftMousePressed(const glm::vec2& mousePosition)
	{
		if (Helper::Collide(GetAbsolutePosition(), Size, mousePosition))
		{
			Checked = !Checked;
			if (Checked)
			{
				for (auto& callback : Callbacks)
					callback(bUICallbackType::Active, *this);
				return true;
			}
		}
		return false;
	}
	
	bUISlider::bUISlider(const glm::vec2& coords, const glm::vec2& size, const glm::vec4& color, const std::string& label, const std::string& name, bUIElementType type)
		:
		bUIElement(coords, size, color, label, name, type),
		Value(0.0f),
		Modified(false)
	{
	}
	void bUISlider::PushQuads(bUIRenderer& renderer, uint32_t& scissorID)
	{
		glm::vec2 handlePosition = GetAbsolutePosition() + glm::vec2((Size.x - Size.y) * Value , 0.0f);
		renderer.Submit<bUISlider>(
			*this, 
			scissorID, 
			bUI::GetContext().Config.GetSubTexture(bUIConfig::Slider),
			bUI::GetContext().Config.GetSubTexture(bUIConfig::SliderHandle),
			handlePosition,
			Value
		);
	}
	bool bUISlider::OnMouseMoved(const glm::vec2& mousePosition)
	{
		if (Modified)
		{
			glm::vec2 absolutePosition = GetAbsolutePosition();
			Value = (mousePosition.x - absolutePosition.x) / Size.x;
			Value = std::clamp(Value, 0.0f, 1.0f);
			ActiveColor = bUI::GetConfig().GetColor(bUIConfig::HighlightColor);
			for (auto& callback : Callbacks)
				callback(bUICallbackType::Active, *this);
		}
		else if (Helper::Collide(GetAbsolutePosition(), Size, mousePosition))
		{
			ActiveColor = bUI::GetConfig().GetColor(bUIConfig::HighlightColor);
			for (auto& callback : Callbacks)
				callback(bUICallbackType::Hoover, *this);
			return true;
		}
		ActiveColor = Color;
		return false;
	}

	bool bUISlider::OnLeftMousePressed(const glm::vec2& mousePosition)
	{
		glm::vec2 handleSize = { Size.y, Size.y };
		if (Helper::Collide(GetAbsolutePosition(), Size, mousePosition))
		{
			Modified = true;
			return true;
		}
		return false;
	}
	bool bUISlider::OnLeftMouseReleased()
	{
		bool old = Modified;
		Modified = false;
		return old;
	}
	bUIWindow::bUIWindow(const glm::vec2& coords, const glm::vec2& size, const glm::vec4& color,  const std::string& label, const std::string& name, bUIElementType type)
		:
		bUIElement(coords, size, color,  label, name, type)
	{
	}
	void bUIWindow::PushQuads(bUIRenderer& renderer, uint32_t& scissorID)
	{
		renderer.Submit<bUIWindow>(
			*this, 
			scissorID, 
			bUI::GetContext().Config.GetSubTexture(bUIConfig::Button), 
			bUI::GetContext().Config.GetSubTexture(bUIConfig::MinimizeButton)
		);
	}
	bool bUIWindow::OnMouseMoved(const glm::vec2& mousePosition)
	{
		glm::vec2 panelSize = { Size.x, ButtonSize.y };
		glm::vec2 absolutePosition = GetAbsolutePosition();
		glm::vec2 absolutePanelPosition = absolutePosition - glm::vec2(0.0f, panelSize.y);
		if (Helper::Collide(absolutePanelPosition, {Size.x, ButtonSize.y}, mousePosition))
		{
			ActiveColor = bUI::GetConfig().GetColor(bUIConfig::HighlightColor);
			for (auto& callback : Callbacks)
				callback(bUICallbackType::Hoover, *this);
			return true;
		}
		else if (ResizeFlags)
		{
			if (IS_SET(ResizeFlags, Right))
			{
				Size.x = mousePosition.x - absolutePosition.x;
			}
			else if (IS_SET(ResizeFlags, Left))
			{
				Size.x = absolutePosition.x + Size.x - mousePosition.x;
				Coords.x = mousePosition.x;
			}
			if (IS_SET(ResizeFlags, Bottom))
			{
				Size.y = mousePosition.y - absolutePosition.y;
			}
			return true;
		}

		ActiveColor = Color;
		return false;
	}
	bool bUIWindow::OnLeftMousePressed(const glm::vec2& mousePosition)
	{
		glm::vec2 panelSize = { Size.x, ButtonSize.y };
		glm::vec2 absolutePosition = GetAbsolutePosition();
		glm::vec2 absolutePanelPosition = absolutePosition - glm::vec2(0.0f, panelSize.y);
		if (Helper::Collide(absolutePanelPosition, ButtonSize, mousePosition))
		{
			ChildrenVisible = !ChildrenVisible;
			for (auto& callback : Callbacks)
				callback(bUICallbackType::Active, *this);
			return true;
		}
		else if (Helper::Collide(absolutePosition, Size, mousePosition))
		{
			if (mousePosition.x > absolutePosition.x + Size.x - sc_ResizeOffset.x)
				ResizeFlags |= Right;
			else if (mousePosition.x < absolutePosition.x + sc_ResizeOffset.x)
				ResizeFlags |= Left;
			if (mousePosition.y > absolutePosition.y + Size.y - sc_ResizeOffset.y)
				ResizeFlags |= Bottom;
			
			return ResizeFlags;
		}
		return false;
	}
	bool bUIWindow::OnRightMousePressed(const glm::vec2& mousePosition)
	{
		glm::vec2 panelSize = { Size.x, ButtonSize.y };
		glm::vec2 absolutePosition = GetAbsolutePosition() - glm::vec2(0.0f, panelSize.y);
		return Helper::Collide(absolutePosition, panelSize, mousePosition);
	}
	bUIScrollbox::bUIScrollbox(const glm::vec2& coords, const glm::vec2& size, const glm::vec4& color, const std::string& label, const std::string& name, bUIElementType type)
		:
		bUIElement(coords, size, color,  label, name, type),
		Offset(0.0f)
	{
	}
	void bUIScrollbox::PushQuads(bUIRenderer& renderer, uint32_t& scissorID)
	{
		renderer.Submit(*this, scissorID, bUI::GetConfig().GetSubTexture(bUIConfig::White));
		scissorID = renderer.GetMesh().Scissors.size() - 1;
	}

	void bUIScrollbox::OnUpdate()
	{
		if (FitParent && Parent)
		{
			Coords = glm::vec2(0.0f);
			Size = Parent->Size;
		}
	}

	bool bUIScrollbox::OnMouseScrolled(const glm::vec2& mousePosition, const glm::vec2& offset)
	{
		if (Helper::Collide(GetAbsoluteScrollPosition(), Size, mousePosition))
		{
			Offset += offset * Speed;
			return true;
		}
		return false;
	}

	glm::vec2 bUIScrollbox::GetAbsolutePosition() const
	{
		if (Parent)
		{
			return Parent->GetAbsolutePosition() + Coords + Offset;
		}
		return Coords;
	}

	glm::vec2 bUIScrollbox::GetAbsoluteScrollPosition() const
	{
		if (Parent)
		{
			return Parent->GetAbsolutePosition() + Coords;
		}
		return Coords;
	}
	
}