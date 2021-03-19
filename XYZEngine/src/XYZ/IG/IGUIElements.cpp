#include "stdafx.h"
#include "IGUIElements.h"
#include "XYZ/Core/KeyCodes.h"

namespace XYZ {
	namespace Helper {
		static bool Collide(const glm::vec2& pos, const glm::vec2& size, const glm::vec2& point)
		{
			return (pos.x + size.x > point.x &&
				pos.x		   < point.x&&
				pos.y + size.y >  point.y &&
				pos.y < point.y);
		}

		static uint32_t FindNumCharacters(const char* source, float maxWidth, Ref<Font> font)
		{
			if (!source)
				return 0;

			float xCursor = 0.0f;
			uint32_t counter = 0;
			while (source[counter] != '\0')
			{
				auto& character = font->GetCharacter(source[counter]);
				if (xCursor + (float)character.XAdvance >= maxWidth)
					break;

				xCursor += character.XAdvance;
				counter++;
			}
			return counter;
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
		bool handled = false;
		glm::vec2 absolutePosition = GetAbsolutePosition();
		if (Helper::Collide(absolutePosition, Size, mousePosition))
		{
			ReturnType = IGReturnType::Clicked;
			glm::vec2 minButtonPos = { absolutePosition.x + Size.x - IGWindow::PanelHeight, absolutePosition.y };
			if (absolutePosition.y + IGWindow::PanelHeight >= mousePosition.y)
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
				handled = true;
			}
			else
			{
				if (mousePosition.x < absolutePosition.x + 5.0f) // Left resize
				{
					Flags |= LeftResize;
					handled = true;
				}
				else if (mousePosition.x > absolutePosition.x + Size.x - 5.0f) // Right resize
				{
					Flags |= RightResize;
					handled = true;
				}
				if (mousePosition.y > absolutePosition.y + Size.y - 5.0f) // Bottom
				{
					Flags |= BottomResize;
					handled = true;
				}
			}
		}
		return handled;
	}

	bool IGWindow::OnLeftRelease(const glm::vec2& mousePosition)
	{
		Flags &= ~Moved;
		Flags &= ~(LeftResize | RightResize | BottomResize);
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

	IGFloat::IGFloat(const glm::vec2& position, const glm::vec2& size, const glm::vec4& color)
		:
		IGElement(position, size, color)
	{
		memset(Buffer, 0, BufferSize);
		snprintf(Buffer, sizeof(Buffer), "%f", Value);
		ModifiedIndex = 0;
		while (Buffer[ModifiedIndex] != '\0')
			ModifiedIndex++;
	}
	bool IGFloat::OnLeftClick(const glm::vec2& mousePosition)
	{
		Listen = false;
		Color = IGRenderData::Colors[IGRenderData::DefaultColor];
		if (Helper::Collide(GetAbsolutePosition(), Size, mousePosition))
		{
			ReturnType = IGReturnType::Clicked;
			Listen = true;
			Color = IGRenderData::Colors[IGRenderData::HooverColor];
			return true;
		}
		return false;
	}
	bool IGFloat::OnMouseMove(const glm::vec2& mousePosition)
	{
		if (!Listen)
			Color = IGRenderData::Colors[IGRenderData::DefaultColor];
		if (Helper::Collide(GetAbsolutePosition(), Size, mousePosition))
		{
			ReturnType = IGReturnType::Hoovered;
			Color = IGRenderData::Colors[IGRenderData::HooverColor];
			return true;
		}
		return false;
	}
	bool IGFloat::OnKeyType(char character)
	{
		if (Listen)
		{
			if (character >= toascii('0') && character <= toascii('9') || character == toascii('.'))
			{
				Buffer[ModifiedIndex++] = character;
				return true;
			}
		}
		return false;
	}
	bool IGFloat::OnKeyPress(int32_t mode, int32_t key)
	{
		if (Listen)
		{
			if (key == ToUnderlying(KeyCode::KEY_BACKSPACE))
			{
				if (ModifiedIndex > 0)
					ModifiedIndex--;
				Buffer[ModifiedIndex] = '\0';
				return true;
			}
		}
		return false;
	}
	glm::vec2 IGFloat::GenerateQuads(IGMesh& mesh, IGRenderData& renderData)
	{
		float textWidth = Size.x - Style.Layout.LeftPadding - Style.Layout.RightPadding;
		size_t numChar = Helper::FindNumCharacters(Buffer, textWidth, renderData.Font);
		Buffer[numChar] = '\0';
		ModifiedIndex = numChar;

		IGMeshFactoryData data = { IGRenderData::Slider, this, &mesh, &renderData };
		return IGMeshFactory::GenerateUI<IGFloat>(Label.c_str(), Color, data);
	}

	float IGFloat::GetValue() const
	{
		return (float)atof(Buffer);
	}


	IGTree::IGTree(const glm::vec2& position, const glm::vec2& size, const glm::vec4& color)
		:
		IGElement(position, size, color),
		Pool(sizeof(IGTreeItem)* NumberOfItemsPerBlockInPool)
	{
	}

	bool IGTree::OnLeftClick(const glm::vec2& mousePosition)
	{
		Hierarchy.Traverse([&](void* parent, void* child) ->bool {
			IGTreeItem* childItem = static_cast<IGTreeItem*>(child);
			if (Helper::Collide(childItem->Position, Size, mousePosition))
			{
				childItem->Open = !childItem->Open;
			}
			return false;
		});
		return false;
	}
	bool IGTree::OnMouseMove(const glm::vec2& mousePosition)
	{
		Hierarchy.Traverse([&](void* parent, void* child) ->bool {
			IGTreeItem* childItem = static_cast<IGTreeItem*>(child);
			childItem->Color = IGRenderData::Colors[IGRenderData::DefaultColor];
			if (Helper::Collide(childItem->Position, Size, mousePosition))
			{
				childItem->Color = IGRenderData::Colors[IGRenderData::HooverColor];
			}
			return false;
		});
		return false;
	}
	glm::vec2 IGTree::GenerateQuads(IGMesh& mesh, IGRenderData& renderData)
	{
		IGMeshFactoryData data = { IGRenderData::RightArrow, this, &mesh, &renderData };
		return IGMeshFactory::GenerateUI<IGTree>(Label.c_str(), Color, data);
	}
	IGTree::IGTreeItem& IGTree::GetItem(const char* name)
	{
		auto it = NameIDMap.find(name);
		XYZ_ASSERT(it != NameIDMap.end(), "");
		
		return *static_cast<IGTreeItem*>(Hierarchy.GetData(it->second));
	}
	void IGTree::AddItem(const char* name, const char* parent, const IGTreeItem& item)
	{
		auto it = NameIDMap.find(name);
		if (it == NameIDMap.end())
		{
			IGTreeItem* ptr = Pool.Allocate<IGTreeItem>(item.Label);
			if (parent)
			{
				auto parentIt = NameIDMap.find(parent);
				if (parentIt != NameIDMap.end())
				{
					ptr->ID = Hierarchy.Insert(ptr, parentIt->second);
					NameIDMap[name] = ptr->ID;
				}
				else
				{
					XYZ_LOG_WARN("Parent item with the name: ", parent, "does not exist");
				}
			}
			else
			{
				ptr->ID = Hierarchy.Insert(ptr);
				NameIDMap[name] = ptr->ID;
			}
		}
		else
		{
			XYZ_LOG_WARN("Item with the name: ", name, "already exists");
		}
	}
	void IGTree::RemoveItem(const char* name)
	{
		auto it = NameIDMap.find(name);
		if (it != NameIDMap.end())
		{
			IGTreeItem* ptr = static_cast<IGTreeItem*>(Hierarchy.GetData(it->second));
			Pool.Deallocate(ptr);
			Hierarchy.Remove(it->second);
			NameIDMap.erase(it);
		}
		else
		{
			XYZ_LOG_WARN("Item with the name: ", name, "does not exist");
		}
	}
	IGGroup::IGGroup(const glm::vec2& position, const glm::vec2& size, const glm::vec4& color)
		: IGElement(position, size, color)
	{
		ActiveChildren = Open;
	}
	bool IGGroup::OnLeftClick(const glm::vec2& mousePosition)
	{
		if (Helper::Collide(GetAbsolutePosition(), Size, mousePosition))
		{
			Open = !Open;
			ActiveChildren = Open;
			return true;
		}
		return false;
	}
	bool IGGroup::OnMouseMove(const glm::vec2& mousePosition)
	{
		return false;
	}
	glm::vec2 IGGroup::GenerateQuads(IGMesh& mesh, IGRenderData& renderData)
	{
		IGMeshFactoryData data = { IGRenderData::Button, this, &mesh, &renderData };
		return IGMeshFactory::GenerateUI<IGGroup>(Label.c_str(), Color, data);
	}
}