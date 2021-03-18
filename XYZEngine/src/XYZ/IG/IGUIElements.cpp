#include "stdafx.h"
#include "IGUIElements.h"

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
	IGWindow::IGWindow(const glm::vec2& position, const glm::vec2& size, const glm::vec4& color)
		:
		IGElement(position, size, color)
	{
		ElementType = IGElementType::Window;
	}

	bool IGWindow::OnLeftClick(const glm::vec2& mousePosition)
	{
		if (Helper::Collide(GetAbsolutePosition(), Size, mousePosition))
		{
			ReturnType = IGReturnType::Clicked;
			glm::vec2 minButtonPos = { Position.x + Size.x - IGWindow::PanelHeight, Position.y };
			if (Position.y + IGWindow::PanelHeight >= mousePosition.y)
			{
				if (Helper::Collide(minButtonPos, { IGWindow::PanelHeight, IGWindow::PanelHeight }, mousePosition))
				{
					Flags ^= IGWindow::Flags::Collapsed;
					ActiveChildren = !IS_SET(Flags, IGWindow::Flags::Collapsed);
				}
				else
				{
					Flags |= IGWindow::Flags::Moved;
				}
				return true;
			}
			else
			{
				// TODO: Resizing
			}
		}
		return false;
	}

	bool IGWindow::OnLeftRelease(const glm::vec2& mousePosition)
	{
		Flags &= ~IGWindow::Flags::Moved;
		if (Helper::Collide(GetAbsolutePosition(), Size, mousePosition))
		{
			ReturnType = IGReturnType::Released;
			return true;
		}
		return false;
	}

	bool IGWindow::OnMouseMove(const glm::vec2& mousePosition)
	{
		if (Helper::Collide(GetAbsolutePosition(), Size, mousePosition))
		{
			ReturnType = IGReturnType::Hoovered;
			return true;
		}
		return false;
	}

	glm::vec2 IGWindow::GenerateQuads(IGMesh& mesh, IGRenderData& renderData)
	{
		IGMeshFactoryData data = { IGRenderData::Window, this, &mesh, &renderData };
		return IGMeshFactory::GenerateUI<IGWindow>(Label.c_str(), glm::vec4(1.0f), data);
	}

	IGButton::IGButton(const glm::vec2& position, const glm::vec2& size, const glm::vec4& color)
		:
		IGElement(position, size, color)
	{}

	bool IGButton::OnLeftClick(const glm::vec2& mousePosition)
	{
		if (Helper::Collide(GetAbsolutePosition(), Size, mousePosition))
		{
			ReturnType = IGReturnType::Clicked;
			return true;
		}
		return false;
	}
	bool IGButton::OnMouseMove(const glm::vec2& mousePosition)
	{
		Color = IGRenderData::Colors[IGRenderData::DefaultColor];
		if (Helper::Collide(GetAbsolutePosition(), Size, mousePosition))
		{
			ReturnType = IGReturnType::Hoovered;
			Color = IGRenderData::Colors[IGRenderData::HooverColor];
			return true;
		}
		return false;
	}
	glm::vec2 IGButton::GenerateQuads(IGMesh& mesh, IGRenderData& renderData)
	{
		IGMeshFactoryData data = { IGRenderData::Button, this, &mesh, &renderData };
		return IGMeshFactory::GenerateUI<IGButton>(Label.c_str(), glm::vec4(1.0f), data);
	}

	IGCheckbox::IGCheckbox(const glm::vec2& position, const glm::vec2& size, const glm::vec4& color)
		:
		IGElement(position, size, color)
	{
	}
	bool IGCheckbox::OnLeftClick(const glm::vec2& mousePosition)
	{
		if (Helper::Collide(GetAbsolutePosition(), Size, mousePosition))
		{
			ReturnType = IGReturnType::Clicked;
			Checked = !Checked;
			return true;
		}
		return false;
	}
	bool IGCheckbox::OnMouseMove(const glm::vec2& mousePosition)
	{
		Color = IGRenderData::Colors[IGRenderData::DefaultColor];
		if (Helper::Collide(GetAbsolutePosition(), Size, mousePosition))
		{
			ReturnType = IGReturnType::Hoovered;
			Color = IGRenderData::Colors[IGRenderData::HooverColor];
			return true;
		}
		return false;
	}
	glm::vec2 IGCheckbox::GenerateQuads(IGMesh& mesh, IGRenderData& renderData)
	{
		uint32_t subTextureIndex = IGRenderData::CheckboxUnChecked;
		if (Checked)
			subTextureIndex = IGRenderData::CheckboxChecked;

		IGMeshFactoryData data = { subTextureIndex, this, &mesh, &renderData };
		return IGMeshFactory::GenerateUI<IGCheckbox>(Label.c_str(), glm::vec4(1.0f), data);
	}
	IGSlider::IGSlider(const glm::vec2& position, const glm::vec2& size, const glm::vec4& color)
		:
		IGElement(position, size, color)
	{
	}
	bool IGSlider::OnLeftClick(const glm::vec2& mousePosition)
	{
		if (Helper::Collide(GetAbsolutePosition(), Size, mousePosition))
		{
			ReturnType = IGReturnType::Clicked;
			Modified = true;
			return true;
		}
		return false;
	}
	bool IGSlider::OnLeftRelease(const glm::vec2& mousePosition)
	{
		Modified = false;
		return false;
	}
	bool IGSlider::OnMouseMove(const glm::vec2& mousePosition)
	{
		Color = IGRenderData::Colors[IGRenderData::DefaultColor];
		if (Modified)
		{
			glm::vec2 absolutePosition = GetAbsolutePosition();
			Value = (mousePosition.x - absolutePosition.x) / Size.x;
			Value = std::clamp(Value, 0.0f, 1.0f);
		}
		if (Helper::Collide(GetAbsolutePosition(), Size, mousePosition))
		{
			ReturnType = IGReturnType::Hoovered;
			Color = IGRenderData::Colors[IGRenderData::HooverColor];
			return true;
		}
	}
	glm::vec2 IGSlider::GenerateQuads(IGMesh& mesh, IGRenderData& renderData)
	{
		IGMeshFactoryData data = { IGRenderData::Slider, this, &mesh, &renderData };
		return IGMeshFactory::GenerateUI<IGSlider>(Label.c_str(), Color, data);
	}
	IGText::IGText(const glm::vec2& position, const glm::vec2& size, const glm::vec4& color)
		:
		IGElement(position, size, color)
	{
	}
	bool IGText::OnLeftClick(const glm::vec2& mousePosition)
	{
		return false;
	}
	bool IGText::OnMouseMove(const glm::vec2& mousePosition)
	{
		Color = IGRenderData::Colors[IGRenderData::DefaultColor];
		if (Helper::Collide(GetAbsolutePosition(), Size, mousePosition))
		{
			ReturnType = IGReturnType::Hoovered;
			Color = IGRenderData::Colors[IGRenderData::HooverColor];
			return true;
		}
		return false;
	}
	glm::vec2 IGText::GenerateQuads(IGMesh& mesh, IGRenderData& renderData)
	{
		IGMeshFactoryData data = { 0, this, &mesh, &renderData };
		return IGMeshFactory::GenerateUI<IGText>(Text.c_str(), Color, data);
	}
}