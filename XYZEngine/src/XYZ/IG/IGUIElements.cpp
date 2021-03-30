#include "stdafx.h"
#include "IGUIElements.h"
#include "IGAllocator.h"

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
		static bool ResolvePosition(size_t oldQuadCount, const glm::vec2& genSize, IGElement* element, const IGElement& root, IGMesh& mesh, glm::vec2& offset, float& highestInRow)
		{
			if (root.Style.AutoPosition)
			{
				if (offset.y + genSize.y > root.Size.y)
				{
					mesh.Quads.erase(mesh.Quads.begin() + oldQuadCount, mesh.Quads.end());
					return false;
				}
				if (offset.x + genSize.x > root.Size.x)
				{
					if (!root.Style.NewRow)
					{
						offset.x += genSize.x + root.Style.Layout.SpacingX;
						mesh.Quads.erase(mesh.Quads.begin() + oldQuadCount, mesh.Quads.end());
						return false;
					}
					else
					{
						offset.x = root.Style.Layout.LeftPadding;
						offset.y += root.Style.Layout.SpacingY + highestInRow;
						highestInRow = 0.0f;
						// It does not fit to the new row
						if (offset.y + genSize.y > root.Size.y)
						{
							mesh.Quads.erase(mesh.Quads.begin() + oldQuadCount, mesh.Quads.end());
							return false;
						}
						// It is generally bigger than xBorder erase it
						if (offset.x + genSize.x > root.Size.x)
						{
							mesh.Quads.erase(mesh.Quads.begin() + oldQuadCount, mesh.Quads.end());
							return false;
						}
						for (size_t i = oldQuadCount; i < mesh.Quads.size(); ++i)
						{
							mesh.Quads[i].Position.x += offset.x - element->Position.x;
							mesh.Quads[i].Position.y += offset.y - element->Position.y;
						}
					}
				}		
			}
			element->Position = offset;			
			offset.x += genSize.x + root.Style.Layout.SpacingX;
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
		if (Helper::Collide(GetAbsolutePosition(), Size, mousePosition))
		{
			ReturnType = IGReturnType::Hoovered;
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
		memset(Buffer, 0, BufferSize);
		snprintf(Buffer, sizeof(Buffer), "%f", Value);
		ModifiedIndex = 0;
		while (Buffer[ModifiedIndex] != '\0')
			ModifiedIndex++;
	}
	bool IGFloat::OnLeftClick(const glm::vec2& mousePosition, bool& handled)
	{
		Listen = false;
		Color = IGRenderData::Colors[IGRenderData::DefaultColor];
		if (!handled && Helper::Collide(GetAbsolutePosition(), Size, mousePosition))
		{
			Listen = true;
			handled = true;
			ReturnType = IGReturnType::Clicked;
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
			if (character >= toascii('0') && character <= toascii('9') || character == toascii('.'))
			{
				Buffer[ModifiedIndex++] = character;
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
		size_t numChar = Helper::FindNumCharacters(Buffer, textWidth, renderData.Font);
		Buffer[numChar] = '\0';
		ModifiedIndex = numChar;

		IGMeshFactoryData data = {renderData.SubTextures[IGRenderData::Slider], this, &mesh, &renderData, scissorIndex };
		return IGMeshFactory::GenerateUI<IGFloat>(Label.c_str(), Color, data);
	}

	float IGFloat::GetValue() const
	{
		return (float)atof(Buffer);
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
			if (!handled && Helper::Collide(childItem->Position, Size, mousePosition))
			{
				childItem->Open = !childItem->Open;
				handled = true;
				return true;
			}
			return false;
		});
		return false;
	}
	bool IGTree::OnMouseMove(const glm::vec2& mousePosition, bool& handled)
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
	glm::vec2 IGTree::GenerateQuads(IGMesh& mesh, IGRenderData& renderData, uint32_t scissorIndex)
	{
		IGMeshFactoryData data = { renderData.SubTextures[IGRenderData::RightArrow], this, &mesh, &renderData, scissorIndex };
		return IGMeshFactory::GenerateUI<IGTree>(Label.c_str(), Color, data);
	}

	IGTreeItem& IGTree::GetItem(const char* name)
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
			Size.y = IGWindow::PanelHeight;
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
	}

	glm::vec2 IGSeparator::GenerateQuads(IGMesh& mesh, IGRenderData& renderData, uint32_t scissorIndex)
	{
		if (AdjustToParent && Parent)
		{
			Size.x = Parent->Size.x - Parent->Style.Layout.LeftPadding - Parent->Style.Layout.RightPadding;
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
		}

		IGMeshFactoryData data = { renderData.SubTextures[IGRenderData::White], this, &mesh, &renderData };

		auto [width, height] = Input::GetWindowSize();
		glm::vec2 absolutePos = GetAbsolutePosition();
		absolutePos.y = height - absolutePos.y - Size.y;

		renderData.Scissors.push_back({ absolutePos.x, absolutePos.y, Size.x, Size.y });
		return IGMeshFactory::GenerateUI<IGScrollbox>(Label.c_str(), Color, data);
	}

	glm::vec2 IGScrollbox::BuildMesh(IGMesh& mesh, IGRenderData& renderData, IGPool& pool, const IGElement& root, uint32_t scissorIndex)
	{
		if (Active && ActiveChildren)
		{	
			glm::vec2 offset = {
				Style.Layout.LeftPadding + Offset.x,
				Style.Layout.TopPadding  + Offset.y
			};
			uint32_t scissorIndex = renderData.Scissors.size() - 1;
			float highestInRow = 0.0f;
			pool.GetHierarchy().TraverseNodeChildren(ID, [&](void* parent, void* child) -> bool {
	
				IGElement* childElement = static_cast<IGElement*>(child);

				size_t oldQuadCount = renderData.ScrollableMesh.Quads.size();
				glm::vec2 genSize = childElement->GenerateQuads(renderData.ScrollableMesh, renderData, scissorIndex );
				if (Helper::ResolvePosition(oldQuadCount, genSize, childElement, root, renderData.ScrollableMesh, offset, highestInRow))
				{				
					childElement->ListenToInput = Helper::IsInside(GetAbsolutePosition(), Size, childElement->GetAbsolutePosition(), childElement->Size);
					highestInRow = std::max(genSize.y, highestInRow);
					offset += childElement->BuildMesh(renderData.ScrollableMesh, renderData, pool, *this, scissorIndex);
				}
				else
				{
					childElement->ListenToInput = false;
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

}