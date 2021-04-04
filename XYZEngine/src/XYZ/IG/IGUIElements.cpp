#include "stdafx.h"
#include "IGUIElements.h"
#include "IGAllocator.h"
#include "IGDockspace.h"

#include "XYZ/Core/KeyCodes.h"
#include "XYZ/Core/Input.h"

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
		static bool ResolvePosition(
			IGElement* element, 
			const glm::vec2& border, const glm::vec2& genSize, const IGStyle& style, 
			IGMesh& mesh, glm::vec2& offset, float& maxY, 
			size_t oldQuadCount, size_t oldLineCount)
		{		
			if (style.AutoPosition)
			{
				if (offset.x + genSize.x > border.x)
				{
					if (offset.y + genSize.y > border.y)
					{
						mesh.Quads.erase(mesh.Quads.begin() + oldQuadCount, mesh.Quads.end());
						mesh.Lines.erase(mesh.Lines.begin() + oldLineCount, mesh.Lines.end());
						return false;
					}
					else if (!style.NewRow)
					{
						offset.x += genSize.x + style.Layout.SpacingX;
						mesh.Quads.erase(mesh.Quads.begin() + oldQuadCount, mesh.Quads.end());
						mesh.Lines.erase(mesh.Lines.begin() + oldLineCount, mesh.Lines.end());
						return false;
					}
					else
					{
						offset.x = style.Layout.LeftPadding;
						offset.y += style.Layout.SpacingY + maxY;
						maxY = 0.0f;
						// It is generally bigger than xBorder erase it
						if (offset.x + genSize.x > border.x)
						{
							mesh.Quads.erase(mesh.Quads.begin() + oldQuadCount, mesh.Quads.end());
							mesh.Lines.erase(mesh.Lines.begin() + oldLineCount, mesh.Lines.end());
							return false;
						}
						for (size_t i = oldQuadCount; i < mesh.Quads.size(); ++i)
						{
							mesh.Quads[i].Position.x += offset.x - element->Position.x;
							mesh.Quads[i].Position.y += offset.y - element->Position.y;
						}
					}
				}
				element->Position = offset;
				offset.x += genSize.x + style.Layout.SpacingX;
			}
			return true;
		}

		static bool IsInside(const glm::vec2& parentPos, const glm::vec2& parentSize, const glm::vec2& pos, const glm::vec2& size)
		{
			return !(pos.x < parentPos.x || pos.x + size.x > parentPos.x + parentSize.x
				  || pos.y < parentPos.y || pos.y + size.y > parentPos.y + parentSize.y);
		}
	}

	IGWindow::IGWindow(const glm::vec2& position, const glm::vec2& size, const glm::vec4& color)
		:
		IGElement(position, size, color, IGElementType::Window)
	{
		Style.Layout.TopPadding += IGWindow::PanelHeight;
	}

	IGWindow::~IGWindow()
	{
	}

	bool IGWindow::OnLeftClick(const glm::vec2& mousePosition, bool& handled)
	{
		glm::vec2 absolutePosition = GetAbsolutePosition();
		if (!handled && Helper::Collide(absolutePosition, Size, mousePosition))
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
					Flags &= ~Docked;
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

	bool IGWindow::OnLeftRelease(const glm::vec2& mousePosition, bool& handled)
	{
		if (IS_SET(Flags, (Moved | LeftResize | RightResize | BottomResize)))
		{
			handled = true;
		}
		Flags &= ~Moved;
		Flags &= ~(LeftResize | RightResize | BottomResize);
		if (Helper::Collide(GetAbsolutePosition(), Size, mousePosition))
		{
			ReturnType = IGReturnType::Released;
			return true;
		}
		return false;
	}

	bool IGWindow::OnMouseMove(const glm::vec2& mousePosition, bool& handled)
	{
		Flags &= ~Hoovered;
		if (Helper::Collide(GetAbsolutePosition(), Size, mousePosition))
		{
			ReturnType = IGReturnType::Hoovered;
			Flags |= Hoovered;
			return true;
		}
		return false;
	}

	glm::vec2 IGWindow::GenerateQuads(IGMesh& mesh, IGRenderData& renderData, uint32_t scissorIndex)
	{
		IGMeshFactoryData data = { renderData.SubTextures[IGRenderData::Window], this, &mesh, &renderData };
		return IGMeshFactory::GenerateUI<IGWindow>(Label.c_str(), glm::vec4(1.0f), data);
	}

	void IGWindow::HandleActions(const glm::vec2& mousePosition,const glm::vec2& mouseDiff, bool& handled)
	{
		if (IS_SET(Flags, IGWindow::Moved))
		{
			Position = mouseDiff;
			handled = true;
		}
		else if (IS_SET(Flags, IGWindow::LeftResize))
		{
			Size.x = GetAbsolutePosition().x + Size.x - mousePosition.x;
			Position.x = mousePosition.x;
			handled = true;
			if (ResizeCallback)
				ResizeCallback(Size);
		}
		else if (IS_SET(Flags, IGWindow::RightResize))
		{
			Size.x = mousePosition.x - GetAbsolutePosition().x;
			handled = true;
			if (ResizeCallback)
				ResizeCallback(Size);
		}

		if (IS_SET(Flags, IGWindow::BottomResize))
		{
			Size.y = mousePosition.y - GetAbsolutePosition().y;
			handled = true;
			if (ResizeCallback)
				ResizeCallback(Size);
		}
	}

	IGImageWindow::IGImageWindow(const glm::vec2& position, const glm::vec2& size, const glm::vec4& color)
		:
		IGWindow(position, size, color)
	{
	}
	
	glm::vec2 IGImageWindow::GenerateQuads(IGMesh& mesh, IGRenderData& renderData, uint32_t scissorIndex)
	{
		IGMeshFactoryData data = { SubTexture, this, &mesh, &renderData, scissorIndex };
		return IGMeshFactory::GenerateUI<IGImageWindow>(Label.c_str(), glm::vec4(1.0f), data);
	}

	IGButton::IGButton(const glm::vec2& position, const glm::vec2& size, const glm::vec4& color)
		:
		IGElement(position, size, color, IGElementType::Button)
	{
	}

	bool IGButton::OnLeftClick(const glm::vec2& mousePosition, bool& handled)
	{
		if (!handled && Helper::Collide(GetAbsolutePosition(), Size, mousePosition))
		{
			handled = true;
			ReturnType = IGReturnType::Clicked;
			return true;
		}
		return false;
	}
	bool IGButton::OnMouseMove(const glm::vec2& mousePosition, bool& handled)
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
	glm::vec2 IGButton::GenerateQuads(IGMesh& mesh, IGRenderData& renderData, uint32_t scissorIndex)
	{
		IGMeshFactoryData data = { renderData.SubTextures[IGRenderData::Button], this, &mesh, &renderData, scissorIndex };
		return IGMeshFactory::GenerateUI<IGButton>(Label.c_str(), glm::vec4(1.0f), data);
	}

	

	IGCheckbox::IGCheckbox(const glm::vec2& position, const glm::vec2& size, const glm::vec4& color)
		:
		IGElement(position, size, color, IGElementType::Checkbox)
	{
	}
	bool IGCheckbox::OnLeftClick(const glm::vec2& mousePosition, bool& handled)
	{
		if (!handled && Helper::Collide(GetAbsolutePosition(), Size, mousePosition))
		{
			ReturnType = IGReturnType::Clicked;
			Checked = !Checked;
			handled = true;
			return true;
		}
		return false;
	}
	bool IGCheckbox::OnMouseMove(const glm::vec2& mousePosition, bool& handled)
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
	glm::vec2 IGCheckbox::GenerateQuads(IGMesh& mesh, IGRenderData& renderData, uint32_t scissorIndex)
	{
		uint32_t subTextureIndex = IGRenderData::CheckboxUnChecked;
		if (Checked)
			subTextureIndex = IGRenderData::CheckboxChecked;

		IGMeshFactoryData data = { renderData.SubTextures[subTextureIndex], this, &mesh, &renderData, scissorIndex };
		return IGMeshFactory::GenerateUI<IGCheckbox>(Label.c_str(), glm::vec4(1.0f), data);
	}
	
	IGSlider::IGSlider(const glm::vec2& position, const glm::vec2& size, const glm::vec4& color)
		:
		IGElement(position, size, color, IGElementType::Slider)
	{
	}
	bool IGSlider::OnLeftClick(const glm::vec2& mousePosition, bool& handled)
	{
		if (!handled && Helper::Collide(GetAbsolutePosition(), Size, mousePosition))
		{
			ReturnType = IGReturnType::Clicked;
			Modified = true;
			handled = true;
			return true;
		}
		return false;
	}
	bool IGSlider::OnLeftRelease(const glm::vec2& mousePosition, bool& handled)
	{
		Modified = false;
		return false;
	}
	bool IGSlider::OnMouseMove(const glm::vec2& mousePosition, bool& handled)
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
	glm::vec2 IGSlider::GenerateQuads(IGMesh& mesh, IGRenderData& renderData, uint32_t scissorIndex)
	{
		IGMeshFactoryData data = { renderData.SubTextures[IGRenderData::Slider], this, &mesh, &renderData, scissorIndex };
		return IGMeshFactory::GenerateUI<IGSlider>(Label.c_str(), Color, data);
	}
	
	IGText::IGText(const glm::vec2& position, const glm::vec2& size, const glm::vec4& color)
		:
		IGElement(position, size, color, IGElementType::Text)
	{
	}
	bool IGText::OnLeftClick(const glm::vec2& mousePosition, bool& handled)
	{
		return false;
	}
	bool IGText::OnMouseMove(const glm::vec2& mousePosition, bool& handled)
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
	glm::vec2 IGText::GenerateQuads(IGMesh& mesh, IGRenderData& renderData, uint32_t scissorIndex)
	{
		IGMeshFactoryData data = { nullptr, this, &mesh, &renderData, scissorIndex };
		return IGMeshFactory::GenerateUI<IGText>(Text.c_str(), Color, data);
	}

	IGFloat::IGFloat(const glm::vec2& position, const glm::vec2& size, const glm::vec4& color)
		:
		IGElement(position, size, color, IGElementType::Float)
	{
		SetValue(0.0f);
	}
	bool IGFloat::OnLeftClick(const glm::vec2& mousePosition, bool& handled)
	{
		bool old = Listen;
		Listen = false;
		Color = IGRenderData::Colors[IGRenderData::DefaultColor];
		if (!handled && Helper::Collide(GetAbsolutePosition(), Size, mousePosition))
		{
			Listen = !old;
			handled = true;
			ReturnType = IGReturnType::Clicked;
			if (Listen)
				Color = IGRenderData::Colors[IGRenderData::HooverColor];
			return true;
		}
		return false;
	}
	bool IGFloat::OnMouseMove(const glm::vec2& mousePosition, bool& handled)
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
	bool IGFloat::OnKeyType(char character, bool& handled)
	{
		if (Listen && !handled)
		{
			handled = true;
			if (character >= toascii('0') && character <= toascii('9') 
			|| (character == toascii('-') && ModifiedIndex == 0)
			|| character == toascii('.'))
			{
				Buffer[ModifiedIndex++] = character;
				ReturnType = IGReturnType::Modified;
				return true;
			}
		}
		return false;
	}
	bool IGFloat::OnKeyPress(int32_t mode, int32_t key, bool& handled)
	{
		if (Listen && !handled)
		{
			if (key == ToUnderlying(KeyCode::KEY_BACKSPACE))
			{
				if (ModifiedIndex > 0)
					ModifiedIndex--;
				Buffer[ModifiedIndex] = '\0';
				handled = true;
				return true;
			}
		}
		return false;
	}
	glm::vec2 IGFloat::GenerateQuads(IGMesh& mesh, IGRenderData& renderData, uint32_t scissorIndex)
	{
		float textWidth = Size.x - Style.Layout.LeftPadding - Style.Layout.RightPadding;
		uint32_t numChar = Helper::FindNumCharacters(Buffer, textWidth, renderData.Font);
		Buffer[numChar] = '\0';
		ModifiedIndex = numChar;

		IGMeshFactoryData data = {renderData.SubTextures[IGRenderData::Slider], this, &mesh, &renderData, scissorIndex };
		return IGMeshFactory::GenerateUI<IGFloat>(Label.c_str(), Color, data);
	}

	void IGFloat::SetValue(float val)
	{
		if (!Listen)
		{
			memset(Buffer, 0, BufferSize);
			snprintf(Buffer, sizeof(Buffer), "%f", val);
			ModifiedIndex = 0;
			while (Buffer[ModifiedIndex] != '\0')
				ModifiedIndex++;
		}
	}

	float IGFloat::GetValue() const
	{
		Value = (float)atof(Buffer);
		return Value;
	}



	IGInt::IGInt(const glm::vec2& position, const glm::vec2& size, const glm::vec4& color)
		:
		IGElement(position, size, color, IGElementType::Float)
	{
		SetValue(0);
	}
	bool IGInt::OnLeftClick(const glm::vec2& mousePosition, bool& handled)
	{
		bool old = Listen;
		Listen = false;
		Color = IGRenderData::Colors[IGRenderData::DefaultColor];
		if (!handled && Helper::Collide(GetAbsolutePosition(), Size, mousePosition))
		{
			Listen = !old;
			handled = true;
			ReturnType = IGReturnType::Clicked;
			if (Listen)
				Color = IGRenderData::Colors[IGRenderData::HooverColor];
			return true;
		}
		return false;
	}
	bool IGInt::OnMouseMove(const glm::vec2& mousePosition, bool& handled)
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
	bool IGInt::OnKeyType(char character, bool& handled)
	{
		if (Listen && !handled)
		{
			handled = true;
			if (character >= toascii('0') && character <= toascii('9')
			|| (character == toascii('-') && ModifiedIndex == 0))
			{
				Buffer[ModifiedIndex++] = character;
				ReturnType = IGReturnType::Modified;
				return true;
			}
		}
		return false;
	}
	bool IGInt::OnKeyPress(int32_t mode, int32_t key, bool& handled)
	{
		if (Listen && !handled)
		{
			if (key == ToUnderlying(KeyCode::KEY_BACKSPACE))
			{
				if (ModifiedIndex > 0)
					ModifiedIndex--;
				Buffer[ModifiedIndex] = '\0';
				handled = true;
				return true;
			}
		}
		return false;
	}
	glm::vec2 IGInt::GenerateQuads(IGMesh& mesh, IGRenderData& renderData, uint32_t scissorIndex)
	{
		float textWidth = Size.x - Style.Layout.LeftPadding - Style.Layout.RightPadding;
		size_t numChar = Helper::FindNumCharacters(Buffer, textWidth, renderData.Font);
		Buffer[numChar] = '\0';
		ModifiedIndex = numChar;

		IGMeshFactoryData data = {renderData.SubTextures[IGRenderData::Slider], this, &mesh, &renderData, scissorIndex };
		return IGMeshFactory::GenerateUI<IGInt>(Label.c_str(), Color, data);
	}

	void IGInt::SetValue(int32_t val)
	{
		if (!Listen)
		{
			memset(Buffer, 0, BufferSize);
			snprintf(Buffer, sizeof(Buffer), "%d", val);
			ModifiedIndex = 0;
			while (Buffer[ModifiedIndex] != '\0')
				ModifiedIndex++;
		}
	}

	int32_t IGInt::GetValue() const
	{
		Value = (int32_t)atoi(Buffer);
		return Value;
	}

	IGString::IGString(const glm::vec2& position, const glm::vec2& size, const glm::vec4& color)
		:
		IGElement(position, size, color, IGElementType::Float)
	{
		SetValue("");
	}

	bool IGString::OnLeftClick(const glm::vec2& mousePosition, bool& handled)
	{
		bool old = Listen;
		Listen = false;
		Color = IGRenderData::Colors[IGRenderData::DefaultColor];
		if (!handled && Helper::Collide(GetAbsolutePosition(), Size, mousePosition))
		{
			Listen = !old;
			handled = true;
			ReturnType = IGReturnType::Clicked;
			if (Listen)
				Color = IGRenderData::Colors[IGRenderData::HooverColor];
			return true;
		}
		return false;
	}

	bool IGString::OnMouseMove(const glm::vec2& mousePosition, bool& handled)
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

	bool IGString::OnKeyType(char character, bool& handled)
	{
		if (Listen && !handled)
		{
			handled = true;
			if (character >= toascii('0') && character <= toascii('9')
				|| (character == toascii('-') && ModifiedIndex == 0))
			{
				Buffer[ModifiedIndex++] = character;
				ReturnType = IGReturnType::Modified;
				return true;
			}
		}
		return false;
	}

	bool IGString::OnKeyPress(int32_t mode, int32_t key, bool& handled)
	{
		if (Listen && !handled)
		{
			if (key == ToUnderlying(KeyCode::KEY_BACKSPACE))
			{
				if (ModifiedIndex > 0)
					ModifiedIndex--;
				Buffer[ModifiedIndex] = '\0';
				handled = true;
				return true;
			}
		}
		return false;
	}

	glm::vec2 IGString::GenerateQuads(IGMesh& mesh, IGRenderData& renderData, uint32_t scissorIndex)
	{
		float textWidth = Size.x - Style.Layout.LeftPadding - Style.Layout.RightPadding;
		size_t numChar = Helper::FindNumCharacters(Buffer, textWidth, renderData.Font);
		Buffer[numChar] = '\0';
		ModifiedIndex = numChar;

		IGMeshFactoryData data = {renderData.SubTextures[IGRenderData::Slider], this, &mesh, &renderData, scissorIndex };
		return IGMeshFactory::GenerateUI<IGString>(Label.c_str(), Color, data);
	}

	void IGString::SetValue(const std::string& val)
	{
		if (!Listen)
		{
			memcpy(Buffer, val.c_str(), val.size()); 
			Buffer[val.size()] = '\0';
			ModifiedIndex = 0;
			while (Buffer[ModifiedIndex] != '\0')
				ModifiedIndex++;
		}
	}

	std::string IGString::GetValue() const
	{
		return Buffer;
	}

	IGTree::IGTree(const glm::vec2& position, const glm::vec2& size, const glm::vec4& color)
		:
		IGElement(position, size, color, IGElementType::Tree),
		Pool(sizeof(IGTreeItem)* NumberOfItemsPerBlockInPool)
	{
	}

	bool IGTree::OnLeftClick(const glm::vec2& mousePosition, bool& handled)
	{
		Hierarchy.Traverse([&](void* parent, void* child) ->bool {
			IGTreeItem* childItem = static_cast<IGTreeItem*>(child);
			if (!handled)
			{
				if (Helper::Collide(childItem->Position + glm::vec2(Size.x, 0.0f), childItem->TextSize, mousePosition))
				{
					if (OnSelect)
						OnSelect(childItem->Key);
					handled = true;
				}
				else if (Helper::Collide(childItem->Position, Size, mousePosition))
				{
					childItem->Open = !childItem->Open;
					handled = true;
				}
			}
			return handled;
		});
		return false;
	}
	IGTree::IGTree(IGTree&& other) noexcept
		:
		IGElement(std::move(other)),
		OnSelect(std::move(other.OnSelect)),
		Hierarchy(std::move(other.Hierarchy)),
		Pool(std::move(other.Pool)),
		NameIDMap(std::move(other.NameIDMap))
	{
	}
	bool IGTree::OnMouseMove(const glm::vec2& mousePosition, bool& handled)
	{
		Hierarchy.Traverse([&](void* parent, void* child) ->bool {
			IGTreeItem* childItem = static_cast<IGTreeItem*>(child);
			childItem->Color = IGRenderData::Colors[IGRenderData::DefaultColor];
			if (Helper::Collide(childItem->Position, Size + glm::vec2(childItem->TextSize.x, 0.0f), mousePosition))
			{
				childItem->Color = IGRenderData::Colors[IGRenderData::HooverColor];
			}
			return false;
		});
		return false;
	}
	glm::vec2 IGTree::GenerateQuads(IGMesh& mesh, IGRenderData& renderData, uint32_t scissorIndex)
	{
		IGMeshFactoryData data = { renderData.SubTextures[IGRenderData::RightArrow], this, &mesh, &renderData, scissorIndex };
		return IGMeshFactory::GenerateUI<IGTree>(Label.c_str(), Color, data);
	}


	IGTreeItem& IGTree::GetItem(uint32_t key)
	{
		auto it = NameIDMap.find(key);
		XYZ_ASSERT(it != NameIDMap.end(), "");
		
		return *static_cast<IGTreeItem*>(Hierarchy.GetData(it->second));
	}
	
	void IGTree::AddItem(uint32_t key, uint32_t parent, const IGTreeItem& item)
	{
		auto it = NameIDMap.find(key);
		if (it == NameIDMap.end())
		{
			IGTreeItem* ptr = Pool.Allocate<IGTreeItem>(item.Label);		
			auto parentIt = NameIDMap.find(parent);
			if (parentIt != NameIDMap.end())
			{
				ptr->ID = Hierarchy.Insert(ptr, parentIt->second);
				ptr->Key = key;
				NameIDMap[key] = ptr->ID;
			}
			else
			{
				XYZ_LOG_WARN("Parent item with the key: ", parent, " does not exist");
			}
		}
		else
		{
			XYZ_LOG_WARN("Item with the key: ", key, " already exists");
		}
	}
	void IGTree::AddItem(uint32_t key, const IGTreeItem& item)
	{
		auto it = NameIDMap.find(key);
		if (it == NameIDMap.end())
		{
			IGTreeItem* ptr = Pool.Allocate<IGTreeItem>(item.Label);	
			ptr->ID = Hierarchy.Insert(ptr);
			ptr->Key = key;
			NameIDMap[key] = ptr->ID;
		}
		else
		{
			XYZ_LOG_WARN("Item with the key: ", key, " already exists");
		}
	}
	void IGTree::RemoveItem(uint32_t child)
	{
		auto it = NameIDMap.find(child);
		if (it != NameIDMap.end())
		{
			Hierarchy.TraverseNode(it->second, [&](void* parent, void* child)->bool {
			
				IGTreeItem* childItem = static_cast<IGTreeItem*>(child);
				NameIDMap.erase(childItem->Key);
				Pool.Deallocate(childItem);
				return false;
			});
		}
		else
		{
			XYZ_LOG_WARN("Item with the key: ", child, " does not exist");
		}
	}

	void IGTree::Clear()
	{
		NameIDMap.clear();	
		Hierarchy.Traverse([&](void* parent, void* child) -> bool {
			Pool.Deallocate<IGTreeItem>(static_cast<IGTreeItem*>(child));
			return false;
		});
		Hierarchy.Clear();
	}

	IGGroup::IGGroup(const glm::vec2& position, const glm::vec2& size, const glm::vec4& color)
		: IGElement(position, size, color, IGElementType::Group)
	{
		ActiveChildren = Open;
		Style.Layout.TopPadding += IGGroup::PanelHeight;
	}
	bool IGGroup::OnLeftClick(const glm::vec2& mousePosition, bool& handled)
	{
		if (!handled && Helper::Collide(GetAbsolutePosition(), { Size.x, PanelHeight}, mousePosition))
		{
			Open = !Open;
			ActiveChildren = Open;
			handled = true;
			return true;
		}
		return false;
	}
	bool IGGroup::OnMouseMove(const glm::vec2& mousePosition, bool& handled)
	{
		return false;
	}
	glm::vec2 IGGroup::GenerateQuads(IGMesh& mesh, IGRenderData& renderData, uint32_t scissorIndex)
	{
		if (AdjustToParent && Parent)
		{
			Size.x = Parent->Size.x - Parent->Style.Layout.LeftPadding - Parent->Style.Layout.RightPadding;
			Size.y = IGGroup::PanelHeight;
		}
		ActiveChildren = Open;
		uint32_t subTextureIndex = IGRenderData::RightArrow;
		if (Open)
			subTextureIndex = IGRenderData::DownArrow;
		IGMeshFactoryData data = { renderData.SubTextures[subTextureIndex], this, &mesh, &renderData, scissorIndex };
		return IGMeshFactory::GenerateUI<IGGroup>(Label.c_str(), Color, data);
	}
	

	IGSeparator::IGSeparator(const glm::vec2& position, const glm::vec2& size)
		:
		IGElement(position, size, glm::vec4(1.0f),IGElementType::Separator)
	{
		Flags = AdjustToParent;
	}

	glm::vec2 IGSeparator::GenerateQuads(IGMesh& mesh, IGRenderData& renderData, uint32_t scissorIndex)
	{
		if (IS_SET(Flags, AdjustToParent) && Parent)
		{
			Size.x = Parent->Size.x - Parent->Style.Layout.LeftPadding - Parent->Style.Layout.RightPadding;
			Size.y = 0.0f;
		}
		if (IS_SET(Flags, AdjustToRoot))
		{
			auto root = FindRoot();
			Size.x = root->Size.x - root->Style.Layout.LeftPadding - root->Style.Layout.RightPadding;
		}
		return Size;
	}
	
	IGScrollbox::IGScrollbox(const glm::vec2& position, const glm::vec2& size, const glm::vec4& color)
		:
		IGElement(position, size, color, IGElementType::Scrollbox)
	{
	}

	bool IGScrollbox::OnMouseScroll(const glm::vec2& mousePosition, float offset, bool& handled)
	{
		if (Helper::Collide(GetAbsolutePosition(), Size, mousePosition))
		{
			handled = true;
			Offset.y += offset * ScrollSpeed;
			return true;
		}
		return false;
	}

	glm::vec2 IGScrollbox::GenerateQuads(IGMesh& mesh, IGRenderData& renderData, uint32_t scissorIndex)
	{
		if (AdjustToParent && Parent)
		{
			Size.x = Parent->Size.x - Parent->Style.Layout.LeftPadding - Parent->Style.Layout.RightPadding;
			Size.y = Parent->Size.y - Parent->Style.Layout.BottomPadding - Position.y;
		}
		IGMeshFactoryData data = { renderData.SubTextures[IGRenderData::Window], this, &mesh, &renderData };

		auto [width, height] = Input::GetWindowSize();
		glm::vec2 absolutePos = GetAbsolutePosition();
		absolutePos.y = height - absolutePos.y - Size.y;

		renderData.Scissors.push_back({ absolutePos.x, absolutePos.y, Size.x, Size.y });
		return IGMeshFactory::GenerateUI<IGScrollbox>(Label.c_str(), Color, data);
	}

	glm::vec2 IGScrollbox::BuildMesh(IGElement* root, IGRenderData& renderData, IGPool& pool, IGMesh& mesh, uint32_t scissorIndex)
	{
		if (Active && ActiveChildren)
		{	
			glm::vec2 offset = {
				Style.Layout.LeftPadding + Offset.x,
				Style.Layout.TopPadding  + Offset.y
			};
			uint32_t newScissorIndex = renderData.Scissors.size() - 1;
			float highestInRow = 0.0f;
			bool out = false;

			pool.GetHierarchy().TraverseNodeChildren(ID, [&](void* parent, void* child) -> bool {
	
				IGElement* childElement = static_cast<IGElement*>(child);
				if (childElement->Active)
				{
					if (out)
					{
						childElement->ListenToInput = false;
						return false;
					}
					size_t oldQuadCount = renderData.ScrollableMesh.Quads.size();
					size_t oldLineCount = renderData.ScrollableMesh.Lines.size();
					glm::vec2 genSize = childElement->GenerateQuads(renderData.ScrollableMesh, renderData, newScissorIndex);
					out = !Helper::ResolvePosition(childElement, root->Size, genSize, Style, mesh, offset, highestInRow, oldQuadCount, oldLineCount);
					childElement->ListenToInput = Helper::IsInside(GetAbsolutePosition(), Size, childElement->GetAbsolutePosition(), childElement->Size);
					
					if (!out)
					{				
						highestInRow = std::max(genSize.y, highestInRow);
						offset += childElement->BuildMesh(this, renderData, pool, renderData.ScrollableMesh, newScissorIndex);
					}			
				}
				return false;
			});
		}
		return glm::vec2(0.0f);
	}

	IGImage::IGImage(const glm::vec2& position, const glm::vec2& size, const glm::vec4& color)
		:
		IGElement(position, size, color, IGElementType::Scrollbox)
	{
	}

	glm::vec2 IGImage::GenerateQuads(IGMesh& mesh, IGRenderData& renderData, uint32_t scissorIndex)
	{
		if (SubTexture.Raw())
		{
			IGMeshFactoryData data = { SubTexture, this, &mesh, &renderData, scissorIndex };
			return IGMeshFactory::GenerateUI<IGImage>(Label.c_str(), glm::vec4(1.0f), data);
		}
		return glm::vec2(0.0f);
	}

	bool IGImage::OnLeftClick(const glm::vec2& mousePosition, bool& handled)
	{
		if (!handled && Helper::Collide(GetAbsolutePosition(), Size, mousePosition))
		{
			handled = true;
			ReturnType = IGReturnType::Clicked;
			return true;
		}
		return false;
	}

	bool IGImage::OnMouseMove(const glm::vec2& mousePosition, bool& handled)
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

	IGPack::IGPack(const std::vector<IGHierarchyElement>& elements)
		:
		IGElement(glm::vec2(0.0f), glm::vec2(0.0f), glm::vec4(0.0f), IGElementType::Pack),
		Pool(elements)
	{
		Style.Layout.LeftPadding = 0.0f;
		Style.Layout.RightPadding = 0.0f;
		Style.Layout.TopPadding = 0.0f;
		Style.Layout.BottomPadding = 0.0f;
	}

	void IGPack::Rebuild(const std::vector<IGHierarchyElement>& elements)
	{
		Pool.Rebuild(elements);
		for (size_t i = 0; i < Pool.Size(); ++i)
		{
			IGElement* elem = Pool[i];
			elem->Parent = this;
		}
	}

	glm::vec2 IGPack::BuildMesh(IGElement* root, IGRenderData& renderData, IGPool& pool, IGMesh& mesh, uint32_t scissorIndex)
	{
		if (Active && ActiveChildren)
		{
			glm::vec2 offset(0.0f);
			glm::vec2 oldOffset = offset;
			float highestInRow = 0.0f;
			bool out = false;
			for (size_t i = 0; i < Pool.Size(); ++i)
			{			
				IGElement* element = Pool[i];
				if (out)
				{
					element->ListenToInput = false;
					continue;
				}
				
				size_t oldQuadCount = mesh.Quads.size();
				size_t oldLineCount = mesh.Lines.size();
				glm::vec2 genSize = element->GenerateQuads(mesh, renderData, scissorIndex);
				out = !Helper::ResolvePosition(element, root->Size, genSize, Style, mesh, offset, highestInRow, oldQuadCount, oldLineCount);
				element->ListenToInput = Helper::IsInside(root->GetAbsolutePosition(), root->Size, element->GetAbsolutePosition(), element->Size);
				
				if (!out)
				{
					highestInRow = std::max(genSize.y, highestInRow);
					offset += element->BuildMesh(root, renderData, Pool, mesh, scissorIndex);
				}
			}
			glm::vec2 result = offset - oldOffset;
			result.y += highestInRow;
		}
		return glm::vec2(0.0f);
	}


	glm::vec2 IGPack::GenerateQuads(IGMesh& mesh, IGRenderData& renderData, uint32_t scissorIndex)
	{
		if (Parent)
			Size = Parent->Size;

		return glm::vec2(0.0f);
	}

	size_t IGPack::PoolSize() const
	{
		return Pool.Size();
	}

	Tree& IGPack::GetHierarchy()
	{
		return Pool.GetHierarchy();
	}

	const Tree& IGPack::GetHierarchy() const
	{
		return Pool.GetHierarchy();
	}

	IGElement& IGPack::operator[] (size_t index)
	{
		return *Pool[index];
	}

	bool IGPack::OnLeftClick(const glm::vec2& mousePosition, bool& handled)
	{
		if (Active && ActiveChildren)
		{
			Pool.GetHierarchy().Traverse([&](void* parent, void* child) -> bool {

				IGElement* childElement = static_cast<IGElement*>(child);
				childElement->OnLeftClick(mousePosition, handled);	
				return false;
			});
		}
		return false;
	}

	IGPack::IGPack(IGPack&& other) noexcept
		:
		IGElement(std::move(other)),
		Pool(std::move(other.Pool))
	{
	}

	bool IGPack::OnMouseMove(const glm::vec2& mousePosition, bool& handled)
	{
		if (Active && ActiveChildren)
		{
			Pool.GetHierarchy().Traverse([&](void* parent, void* child) -> bool {
				
				IGElement* childElement = static_cast<IGElement*>(child);
				childElement->OnMouseMove(mousePosition, handled);	
				return false;
			});
		}
		return false;
	}

	bool IGPack::OnLeftRelease(const glm::vec2& mousePosition, bool& handled)
	{
		if (Active && ActiveChildren)
		{
			Pool.GetHierarchy().Traverse([&](void* parent, void* child) -> bool {

				IGElement* childElement = static_cast<IGElement*>(child);
				childElement->OnLeftRelease(mousePosition, handled);			
				return false;	
			});
		}
		return false;
	}

	bool IGPack::OnKeyType(char character, bool& handled)
	{
		if (Active && ActiveChildren)
		{
			Pool.GetHierarchy().Traverse([&](void* parent, void* child) -> bool {

				IGElement* childElement = static_cast<IGElement*>(child);
				childElement->OnKeyType(character, handled);			
				return false;	
			});
		}
		return false;
	}

	bool IGPack::OnKeyPress(int32_t mode, int32_t key, bool& handled)
	{
		if (Active && ActiveChildren)
		{
			Pool.GetHierarchy().Traverse([&](void* parent, void* child) -> bool {

				IGElement* childElement = static_cast<IGElement*>(child);
				childElement->OnKeyPress(mode, key, handled);			
				return false;	
			});
		}
		return false;
	}

	const IGElement& IGPack::operator[] (size_t index) const
	{
		return *Pool[index];
	}
	IGDropdown::IGDropdown(const glm::vec2& position, const glm::vec2& size, const glm::vec4& color)
		:
		IGElement(position, size, color, IGElementType::Dropdown)
	{
	}
}