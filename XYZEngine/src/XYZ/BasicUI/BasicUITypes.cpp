#include "stdafx.h"
#include "BasicUITypes.h"

#include "BasicUIRenderer.h"
#include "BasicUIHelper.h"
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

	bUIListener* bUIListener::s_Selected = nullptr;

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
	void bUIElement::OnUpdate()
	{
	}
	bool bUIElement::OnMouseMoved(const glm::vec2& mousePosition)
	{
		if (!Visible) 
			return false;
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
		if (!Visible) 
			return false;
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
		if (!Visible) 
			return false;
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
	void bUIElement::HandleVisibility(uint32_t scissorID)
	{
		const bUIScissor& scissor = bUI::GetContext().Renderer.GetMesh().Scissors[scissorID];

		glm::vec2 scissorPos = { scissor.X, bUI::GetContext().ViewportSize.y - scissor.Y - scissor.Height };
		glm::vec2 scissorSize = { scissor.Width, scissor.Height };
		glm::vec2 leftTopBorder = scissorPos - Size;
		glm::vec2 rightBottomBorder = scissorPos + scissorSize + Size;
		glm::vec2 size = rightBottomBorder - leftTopBorder;

		Visible = bUIHelper::IsInside(GetAbsolutePosition(), Size, leftTopBorder, size);
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
		HandleVisibility(scissorID);
		if (!Visible)
			return;
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
		HandleVisibility(scissorID);
		if (!Visible)
			return;
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
		if (!Visible) 
			return false;
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
		HandleVisibility(scissorID);
		if (!Visible)
			return;

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
		if (!Visible) 
			return false;
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
		if (!Visible) 
			return false;
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
		if (!Visible) 
			return false;
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
		HandleVisibility(scissorID);
		if (!Visible)
			return;
		renderer.Submit<bUIWindow>(
			*this, 
			scissorID, 
			bUI::GetContext().Config.GetSubTexture(bUIConfig::Button), 
			bUI::GetContext().Config.GetSubTexture(bUIConfig::MinimizeButton)
		);
	}
	bool bUIWindow::OnMouseMoved(const glm::vec2& mousePosition)
	{
		if (!Visible) 
			return false;
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
		if (!Visible) 
			return false;
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
		if (!Visible) 
			return false;
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
		HandleVisibility(scissorID);
		if (!Visible)
			return;

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
		if (!Visible) 
			return false;
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
	
	bUITree::bUITree(const glm::vec2& coords, const glm::vec2& size, const glm::vec4& color, const std::string& label, const std::string& name, bUIElementType type)
		:
		bUIElement(coords, size, color,  label, name, type),
		Pool(sc_NumberOfItemsPerBlockInPool * sizeof(bUITreeItem))
	{
	}

	bUITree::bUITree(bUITree&& other) noexcept
		:
		bUIElement(std::move(other)),
		OnSelect(std::move(other.OnSelect)),
		Hierarchy(std::move(other.Hierarchy)),
		Pool(std::move(other.Pool)),
		NameIDMap(std::move(other.NameIDMap))
	{
	}

	void bUITree::PushQuads(bUIRenderer& renderer, uint32_t& scissorID)
	{
		HandleVisibility(scissorID);
		if (!Visible)
			return;

		solveTreePosition();
		renderer.Submit<bUITree>(*this, scissorID,
			bUI::GetConfig().GetSubTexture(bUIConfig::RightArrow),
			bUI::GetConfig().GetSubTexture(bUIConfig::DownArrow)
		);
	}

	bool bUITree::OnMouseMoved(const glm::vec2& mousePosition)
	{
		glm::vec2 absolutePosition = GetAbsolutePosition();
		Hierarchy.Traverse([&](void* parent, void* child) ->bool {
			if (parent)
			{
				if (!static_cast<bUITreeItem*>(parent)->Open)
					return false;
			}
			bUITreeItem* childItem = static_cast<bUITreeItem*>(child);
			childItem->Color = Color;
			glm::vec2 textSize = bUIHelper::FindTextSize(childItem->Label.c_str(), bUI::GetConfig().GetFont());
			glm::vec2 size = { Size.x + textSize.x, std::max(Size.y, textSize.y) };
			glm::vec2 itemAbsolutePosition = absolutePosition + childItem->Coords;
			if (Helper::Collide(itemAbsolutePosition, size, mousePosition))
			{
				childItem->Color = bUI::GetConfig().GetColor(bUIConfig::HighlightColor);
			}
			return false;
		});
		return false;
	}

	bool bUITree::OnLeftMousePressed(const glm::vec2& mousePosition)
	{
		glm::vec2 absolutePosition = GetAbsolutePosition();
		Hierarchy.Traverse([&](void* parent, void* child) ->bool {
			if (parent)
			{
				if (!static_cast<bUITreeItem*>(parent)->Open)
					return false;
			}
			bUITreeItem* childItem = static_cast<bUITreeItem*>(child);

			glm::vec2 itemAbsolutePosition = absolutePosition + childItem->Coords;
			glm::vec2 textPosition = itemAbsolutePosition + glm::vec2(Size.x, 0.0f);
			glm::vec2 textSize = bUIHelper::FindTextSize(childItem->Label.c_str(), bUI::GetConfig().GetFont());
			if (Helper::Collide(itemAbsolutePosition, Size, mousePosition))
			{
				childItem->Open = !childItem->Open;
				return true;
			}
			if (Helper::Collide(textPosition, textSize, mousePosition))
			{
				if (OnSelect)
					OnSelect(childItem->GetKey());
				return true;
			}
			return false;
		});
		return false;
	}

	bool bUITree::OnRightMousePressed(const glm::vec2& mousePosition)
	{
		bool result = false;
		glm::vec2 absolutePosition = GetAbsolutePosition();
		Hierarchy.Traverse([&](void* parent, void* child) ->bool {
			if (parent)
			{
				if (!static_cast<bUITreeItem*>(parent)->Open)
					return false;
			}
			bUITreeItem* childItem = static_cast<bUITreeItem*>(child);

			glm::vec2 textSize = bUIHelper::FindTextSize(childItem->Label.c_str(), bUI::GetConfig().GetFont());
			glm::vec2 size = { Size.x + textSize.x, std::max(Size.y, textSize.y) };
			glm::vec2 itemAbsolutePosition = absolutePosition + childItem->Coords;
			if (Helper::Collide(itemAbsolutePosition, size, mousePosition))
			{
				result = true;
				return true;
			}
			return false;
		});
		return result;
	}

	void bUITree::AddItem(uint32_t key, uint32_t parent, const bUITreeItem& item)
	{
		auto it = NameIDMap.find(key);
		if (it == NameIDMap.end())
		{
			bUITreeItem* ptr = Pool.Allocate<bUITreeItem>(item.Label);		
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

	void bUITree::AddItem(uint32_t key, const bUITreeItem& item)
	{
		auto it = NameIDMap.find(key);
		if (it == NameIDMap.end())
		{
			bUITreeItem* ptr = Pool.Allocate<bUITreeItem>(item.Label);	
			ptr->ID = Hierarchy.Insert(ptr);
			ptr->Key = key;
			NameIDMap[key] = ptr->ID;
		}
		else
		{
			XYZ_LOG_WARN("Item with the key: ", key, " already exists");
		}
	}

	void bUITree::RemoveItem(uint32_t key)
	{
		auto it = NameIDMap.find(key);
		if (it != NameIDMap.end())
		{
			Hierarchy.TraverseNodeChildren(it->second, [&](void* parent, void* child)->bool {

				bUITreeItem* childItem = static_cast<bUITreeItem*>(child);
				NameIDMap.erase(childItem->Key);
				Hierarchy.Remove(childItem->ID);
				Pool.Deallocate(childItem);
				return false;
				});

			bUITreeItem* item = static_cast<bUITreeItem*>(Hierarchy.GetData(NameIDMap[key]));
			Hierarchy.Remove(item->ID);
			NameIDMap.erase(item->Key);
			Pool.Deallocate(item);
		}
		else
		{
			XYZ_LOG_WARN("Item with the key: ", key, " does not exist");
		}
	}

	void bUITree::Clear()
	{
		NameIDMap.clear();	
		Hierarchy.Traverse([&](void* parent, void* child) -> bool {
			Pool.Deallocate<bUITreeItem>(static_cast<bUITreeItem*>(child));
			return false;
			});
		Hierarchy.Clear();
	}

	bUITreeItem& bUITree::GetItem(uint32_t key)
	{
		auto it = NameIDMap.find(key);
		XYZ_ASSERT(it != NameIDMap.end(), "");

		return *static_cast<bUITreeItem*>(Hierarchy.GetData(it->second));
	}

	void bUITree::solveTreePosition()
	{
		uint32_t currentDepth = 0;
		glm::vec2 offset = glm::vec2(0.0f);
		auto& nodes = Hierarchy.GetFlatNodes();
		Hierarchy.Traverse([&](void* parent, void* child) ->bool {

			bUITreeItem* childItem = static_cast<bUITreeItem*>(child);
			glm::vec2 textSize = bUIHelper::FindTextSize(childItem->Label.c_str(), bUI::GetConfig().GetFont());
			glm::vec2 size = { Size.x + textSize.x, std::max(Size.y, textSize.y) };

			if (nodes[childItem->ID].Depth > currentDepth)
			{
				offset.x += sc_NodeOffset;
			}
			while (nodes[childItem->ID].Depth < currentDepth)
			{
				offset.x -= sc_NodeOffset;
				currentDepth--;
			}
			childItem->Coords = offset;
			bool open = true;
			if (parent)
			{
				bUITreeItem* parentItem = static_cast<bUITreeItem*>(parent);
				open = parentItem->Open;
			}
			if (open)
				offset.y += size.y;
			else
				childItem->Open = false;

			currentDepth = nodes[childItem->ID].Depth;
			return false;
		});
	}

	bUIFloat::bUIFloat(const glm::vec2& coords, const glm::vec2& size, const glm::vec4& color, const std::string& label, const std::string& name, bUIElementType type)
		:
		bUIElement(coords, size, color,  label, name, type)
	{
		SetValue(0.0f);
	}

	void bUIFloat::PushQuads(bUIRenderer& renderer, uint32_t& scissorID)
	{
		HandleVisibility(scissorID);
		if (!Visible)
			return;
		renderer.Submit<bUIFloat>(*this, scissorID, bUI::GetContext().Config.GetSubTexture(bUIConfig::Button));
	}

	bool bUIFloat::OnMouseMoved(const glm::vec2& mousePosition)
	{
		return false;
	}

	bool bUIFloat::OnLeftMousePressed(const glm::vec2& mousePosition)
	{
		if (!Visible) 
			return false;

		bool listen = Listen;
		Listen = false;
		ActiveColor = Color;
		if (Helper::Collide(GetAbsolutePosition(), Size, mousePosition))
		{		
			bUIListener::setListener(this);
			Listen = !listen;
			if (Listen)
				ActiveColor = bUI::GetConfig().GetColor(bUIConfig::HighlightColor);

			for (auto& callback : Callbacks)
				callback(bUICallbackType::Active, *this);
			return true;
		}
		return false;
	}

	bool bUIFloat::OnKeyPressed(int32_t mode, int32_t key)
	{
		if (Listen && Visible)
		{
			if (key == ToUnderlying(KeyCode::KEY_BACKSPACE))
			{
				if (InsertionIndex > 0)
					InsertionIndex--;
				Buffer[InsertionIndex] = '\0';
				for (auto& callback : Callbacks)
					callback(bUICallbackType::Active, *this);
				return true;
			}
		}
		return false;
	}

	bool bUIFloat::OnKeyTyped(char character)
	{
		if (Listen && Visible)
		{
			if (character >= toascii('0') && character <= toascii('9') 
				|| (character == toascii('-') && InsertionIndex == 0)
				|| character == toascii('.'))
			{
				Buffer[InsertionIndex++] = character;
				for (auto& callback : Callbacks)
					callback(bUICallbackType::Active, *this);
				return true;
			}
		}
		return false;
	}

	void bUIFloat::SetValue(float val)
	{
		if (!Listen)
		{
			memset(Buffer, 0, BufferSize);
			snprintf(Buffer, sizeof(Buffer), "%f", val);
			InsertionIndex = 0;
			while (Buffer[InsertionIndex] != '\0')
				InsertionIndex++;
		}
	}

	float bUIFloat::GetValue() const
	{
		Value = (float)atof(Buffer);
		return Value;
	}

	void bUIListener::setListener(bUIListener* listener)
	{
		if (s_Selected)
		{
			s_Selected->Listen = false;
			if (auto casted = dynamic_cast<bUIElement*>(s_Selected))
				casted->ActiveColor = casted->Color;
		}
		s_Selected = listener;
	}

}