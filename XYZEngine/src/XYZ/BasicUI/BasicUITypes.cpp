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
	void bUIButton::PushQuads(bUIRenderer& renderer)
	{
		renderer.Submit<bUIButton>(*this, bUI::GetContext().Config.GetSubTexture(bUIConfig::Button));
	}
	
	bUICheckbox::bUICheckbox(const glm::vec2& coords, const glm::vec2& size, const glm::vec4& color,  const std::string& label, const std::string& name, bUIElementType type)
		:
		bUIElement(coords, size, color,  label, name, type),
		Checked(false)
	{
	}
	void bUICheckbox::PushQuads(bUIRenderer& renderer)
	{
		if (Checked)
			renderer.Submit<bUICheckbox>(*this, bUI::GetContext().Config.GetSubTexture(bUIConfig::CheckboxChecked));
		else
			renderer.Submit<bUICheckbox>(*this, bUI::GetContext().Config.GetSubTexture(bUIConfig::CheckboxUnChecked));
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
	void bUISlider::PushQuads(bUIRenderer& renderer)
	{
		glm::vec2 handlePosition = GetAbsolutePosition() + glm::vec2((Size.x - Size.y) * Value , 0.0f);
		renderer.Submit<bUISlider>(
			*this, 
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
	bUIGroup::bUIGroup(const glm::vec2& coords, const glm::vec2& size, const glm::vec4& color,  const std::string& label, const std::string& name, bUIElementType type)
		:
		bUIElement(coords, size, color,  label, name, type)
	{
	}
	void bUIGroup::PushQuads(bUIRenderer& renderer)
	{
		renderer.Submit<bUIGroup>(
			*this, 
			bUI::GetContext().Config.GetSubTexture(bUIConfig::Button), 
			bUI::GetContext().Config.GetSubTexture(bUIConfig::MinimizeButton)
		);
	}
	bool bUIGroup::OnMouseMoved(const glm::vec2& mousePosition)
	{
		if (Helper::Collide(GetAbsolutePosition(), {Size.x, ButtonSize.y}, mousePosition))
		{
			ActiveColor = bUI::GetConfig().GetColor(bUIConfig::HighlightColor);
			for (auto& callback : Callbacks)
				callback(bUICallbackType::Hoover, *this);
			return true;
		}
		ActiveColor = Color;
		return false;
	}
	bool bUIGroup::OnLeftMousePressed(const glm::vec2& mousePosition)
	{
		if (Helper::Collide(GetAbsolutePosition(), ButtonSize, mousePosition))
		{
			ChildrenVisible = !ChildrenVisible;
			for (auto& callback : Callbacks)
				callback(bUICallbackType::Active, *this);
			return true;
		}
		return false;
	}
	bool bUIGroup::OnRightMousePressed(const glm::vec2& mousePosition)
	{
		return Helper::Collide(GetAbsolutePosition(), { Size.x, ButtonSize.y } , mousePosition);
	}
}