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
		Locked(false),
		BlockEvents(true),
		Type(type)
	{
	}
	bUIElement::bUIElement(const bUIElement& other)
		:
		Coords(other.Coords),
		Size(other.Size),
		Color(other.Color),
		ActiveColor(other.ActiveColor),
		Label(other.Label),
		Name(other.Name),
		Parent(other.Parent),
		Visible(other.Visible),
		ChildrenVisible(other.ChildrenVisible),
		Locked(other.Locked),
		BlockEvents(other.BlockEvents),
		Type(other.Type),
		Callbacks(other.Callbacks),
		ScissorID(other.ScissorID),
		ID(other.ID)
	{
	}
	bUIElement::bUIElement(bUIElement&& other) noexcept
		:
		Coords(other.Coords),
		Size(other.Size),
		Color(other.Color),
		ActiveColor(other.ActiveColor),
		Label(std::move(other.Label)),
		Name(std::move(other.Name)),
		Parent(other.Parent),
		Visible(other.Visible),
		ChildrenVisible(other.ChildrenVisible),
		Locked(other.Locked),
		BlockEvents(other.BlockEvents),
		Type(other.Type),
		Callbacks(std::move(other.Callbacks)),
		ScissorID(other.ScissorID),
		ID(other.ID)
	{
	}
	void bUIElement::OnUpdate()
	{
	}
	bool bUIElement::OnMouseMoved(const glm::vec2& mousePosition)
	{
		if (!Visible || !HandleVisibility(ScissorID))
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
		if (!Visible || !HandleVisibility(ScissorID))
			return false;
		if (Helper::Collide(GetAbsolutePosition(), Size, mousePosition))
		{
			for (auto& callback : Callbacks)
				callback(bUICallbackType::Active, *this);
			return BlockEvents;
		}
		return false;
	}
	bool bUIElement::OnRightMousePressed(const glm::vec2& mousePosition)
	{
		if (!Visible || !HandleVisibility(ScissorID))
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
	glm::vec2 bUIElement::GetSize() const
	{
		return Size;
	}
	bool bUIElement::HandleVisibility(uint32_t scissorID)
	{
		if (bUI::GetContext().Renderer.GetMesh().Scissors.size() <= scissorID)
			return false;
		
		const bUIScissor& scissor = bUI::GetContext().Renderer.GetMesh().Scissors[scissorID];
		glm::vec2 scissorPos = { scissor.X, bUI::GetContext().ViewportSize.y - scissor.Y - scissor.Height };
		glm::vec2 scissorSize = { scissor.Width, scissor.Height };
		glm::vec2 leftTopBorder = scissorPos - GetSize();
		glm::vec2 rightBottomBorder = scissorPos + scissorSize + GetSize();
		glm::vec2 size = rightBottomBorder - leftTopBorder;

		return bUIHelper::IsInside(GetAbsolutePosition(), GetSize(), leftTopBorder, size);
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
	bUIButton::bUIButton(const bUIButton& other)
		:
		bUIElement(other)
	{
	}
	void bUIButton::PushQuads(bUIRenderer& renderer, uint32_t& scissorID)
	{
		ScissorID = scissorID;
		if (!Visible || !HandleVisibility(ScissorID))
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
		ScissorID = scissorID;
		if (!Visible || !HandleVisibility(ScissorID))
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
		if (!Visible || !HandleVisibility(ScissorID))
			return false;
		if (Helper::Collide(GetAbsolutePosition(), Size, mousePosition))
		{
			Checked = !Checked;
			for (auto& callback : Callbacks)
				callback(bUICallbackType::StateChange, *this);

			return BlockEvents;
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
		ScissorID = scissorID;
		if (!Visible || !HandleVisibility(ScissorID))
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
		if (!Visible || !HandleVisibility(ScissorID))
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
			return BlockEvents;
		}
		ActiveColor = Color;
		return false;
	}

	bool bUISlider::OnLeftMousePressed(const glm::vec2& mousePosition)
	{
		if (!Visible || !HandleVisibility(ScissorID))
			return false;
		glm::vec2 handleSize = { Size.y, Size.y };
		if (Helper::Collide(GetAbsolutePosition(), Size, mousePosition))
		{
			Modified = true;
			return BlockEvents;
		}
		return false;
	}
	bool bUISlider::OnLeftMouseReleased()
	{
		if (!Visible || !HandleVisibility(ScissorID))
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
		ScissorID = scissorID;
		if (!Visible || !HandleVisibility(ScissorID))
			return;
		renderer.Submit<bUIWindow>(
			*this, 
			scissorID, 
			bUI::GetContext().Config.GetSubTexture(bUIConfig::Button), 
			bUI::GetContext().Config.GetSubTexture(bUIConfig::MinimizeButton)
		);
	}

	static bool AlmostEqual(float a, float b, float tolerance)
	{
		return fabs(a - b) < tolerance;
	}

	static bool AlmostEqual(const glm::vec2& a, const glm::vec2& b)
	{
		return AlmostEqual(a.x, b.x, 0.1f) && AlmostEqual(a.y, b.y, 0.1f);
	}

	void bUIWindow::OnUpdate()
	{
		if (FitParent && Parent)
		{
			glm::vec2 newSize = Parent->Size - glm::vec2(0.0f, ButtonSize.y);
			if (!AlmostEqual(Size, newSize) && OnResize)
				OnResize(newSize);
			Size = newSize;
		}
	}
	bool bUIWindow::OnMouseMoved(const glm::vec2& mousePosition)
	{
		if (!Visible || !HandleVisibility(ScissorID))
			return false;

		IsHoovered = false;
		glm::vec2 panelSize = { Size.x, ButtonSize.y };
		glm::vec2 absolutePosition = GetAbsolutePosition();
		glm::vec2 absolutePanelPosition = absolutePosition - glm::vec2(0.0f, panelSize.y);
		
		if (Helper::Collide(absolutePanelPosition, { Size.x, Size.y + ButtonSize.y }, mousePosition))
		{
			IsHoovered = true;
			if (mousePosition.y < absolutePosition.y)
			{
				ActiveColor = bUI::GetConfig().GetColor(bUIConfig::HighlightColor);
				for (auto& callback : Callbacks)
					callback(bUICallbackType::Hoover, *this);
				return BlockEvents;
			}
		}
		if (ResizeFlags)
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
			if (OnResize)
				OnResize(Size);
			return BlockEvents;
		}

		ActiveColor = Color;
		return false;
	}
	bool bUIWindow::OnLeftMousePressed(const glm::vec2& mousePosition)
	{
		if (!Visible || !HandleVisibility(ScissorID))
			return false;

		glm::vec2 panelSize = { Size.x, ButtonSize.y };
		glm::vec2 absolutePosition = GetAbsolutePosition();
		glm::vec2 absolutePanelPosition = absolutePosition - glm::vec2(0.0f, panelSize.y);
		if (Helper::Collide(absolutePanelPosition, ButtonSize, mousePosition))
		{
			ChildrenVisible = !ChildrenVisible;
			for (auto& callback : Callbacks)
				callback(bUICallbackType::Active, *this);
			return BlockEvents;
		}
		else if (ChildrenVisible && Helper::Collide(absolutePosition, Size, mousePosition))
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
		if (!Visible || !HandleVisibility(ScissorID))
			return false;

		glm::vec2 panelSize = { Size.x, ButtonSize.y };
		glm::vec2 absolutePosition = GetAbsolutePosition() - glm::vec2(0.0f, panelSize.y);
		return Helper::Collide(absolutePosition, panelSize, mousePosition);
	}
	glm::vec2 bUIWindow::GetSize() const
	{
		if (ChildrenVisible)
			return Size + glm::vec2(0.0f, ButtonSize.y);
		else
			return { Size.x, ButtonSize.y };
	}
	bUIScrollbox::bUIScrollbox(const glm::vec2& coords, const glm::vec2& size, const glm::vec4& color, const std::string& label, const std::string& name, bUIElementType type)
		:
		bUIElement(coords, size, color,  label, name, type),
		Offset(0.0f)
	{
	}
	void bUIScrollbox::PushQuads(bUIRenderer& renderer, uint32_t& scissorID)
	{
		ScissorID = scissorID;
		if (!Visible || !HandleVisibility(ScissorID))
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
		if (!Visible || !EnableScroll || !HandleVisibility(ScissorID)) 
			return false;
		if (Helper::Collide(GetAbsoluteScrollPosition(), Size, mousePosition))
		{
			Offset += offset * Speed;
			return BlockEvents;
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


	bUIHierarchyElement::bUIHierarchyElement()
		:
		Pool(sc_NumberOfItemsPerBlockInPool * sizeof(bUIHierarchyItem))
	{
	}

	bUIHierarchyElement::bUIHierarchyElement(bUIHierarchyElement&& other) noexcept
		:
		Hierarchy(std::move(other.Hierarchy)),
		Pool(std::move(other.Pool)),
		NameIDMap(std::move(other.NameIDMap))
	{
	}

	void bUIHierarchyElement::AddItem(uint32_t key, uint32_t parent, const bUIHierarchyItem& item, bool atEnd)
	{
		auto it = NameIDMap.find(key);
		if (it == NameIDMap.end())
		{
			bUIHierarchyItem* ptr = Pool.Allocate<bUIHierarchyItem>(item.Label);		
			auto parentIt = NameIDMap.find(parent);
			if (parentIt != NameIDMap.end())
			{
				if (atEnd)
					ptr->ID = Hierarchy.InsertAtEnd(ptr, parentIt->second);
				else
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

	void bUIHierarchyElement::AddItem(uint32_t key, const bUIHierarchyItem& item, bool atEnd)
	{
		auto it = NameIDMap.find(key);
		if (it == NameIDMap.end())
		{
			bUIHierarchyItem* ptr = Pool.Allocate<bUIHierarchyItem>(item.Label);	
			if (atEnd)
				ptr->ID = Hierarchy.InsertAtEnd(ptr);
			else
				ptr->ID = Hierarchy.Insert(ptr);
			ptr->Key = key;
			NameIDMap[key] = ptr->ID;
		}
		else
		{
			XYZ_LOG_WARN("Item with the key: ", key, " already exists");
		}
	}

	void bUIHierarchyElement::SetParent(uint32_t key, uint32_t newParent)
	{
		auto child = NameIDMap.find(key);
		auto parent = NameIDMap.find(newParent);
		if (parent == NameIDMap.end())
			Hierarchy.SetParent(child->second, TreeNode::sc_Invalid);
		else
			Hierarchy.SetParent(child->second, parent->second);
	}

	void bUIHierarchyElement::RemoveItem(uint32_t key)
	{
		auto it = NameIDMap.find(key);
		if (it != NameIDMap.end())
		{
			Hierarchy.TraverseNodeChildren(it->second, [&](void* parent, void* child)->bool {

				bUIHierarchyItem* childItem = static_cast<bUIHierarchyItem*>(child);
				NameIDMap.erase(childItem->Key);
				Hierarchy.Remove(childItem->ID);
				Pool.Deallocate(childItem);
				return false;
				});

			bUIHierarchyItem* item = static_cast<bUIHierarchyItem*>(Hierarchy.GetData(NameIDMap[key]));
			Hierarchy.Remove(item->ID);
			NameIDMap.erase(item->Key);
			Pool.Deallocate(item);
		}
		else
		{
			XYZ_LOG_WARN("Item with the key: ", key, " does not exist");
		}
	}

	void bUIHierarchyElement::Clear()
	{
		NameIDMap.clear();	
		Hierarchy.Traverse([&](void* parent, void* child) -> bool {
			Pool.Deallocate<bUIHierarchyItem>(static_cast<bUIHierarchyItem*>(child));
			return false;
			});
		Hierarchy.Clear();
	}

	bUIHierarchyItem& bUIHierarchyElement::GetItem(uint32_t key)
	{
		auto it = NameIDMap.find(key);
		XYZ_ASSERT(it != NameIDMap.end(), "");

		return *static_cast<bUIHierarchyItem*>(Hierarchy.GetData(it->second));
	}

	void bUIHierarchyElement::solveTreePosition(const glm::vec2& size)
	{
		uint32_t currentDepth = 0;
		glm::vec2 offset = glm::vec2(0.0f);
		auto& nodes = Hierarchy.GetFlatNodes();
		Hierarchy.Traverse([&](void* parent, void* child) ->bool {

			bUIHierarchyItem* childItem = static_cast<bUIHierarchyItem*>(child);
			glm::vec2 textSize = bUIHelper::FindTextSize(childItem->Label.c_str(), bUI::GetConfig().GetFont());
			glm::vec2 tmpSize = { size.x + textSize.x, std::max(size.y, textSize.y) };

			if (nodes[childItem->GetID()].Depth > currentDepth)
			{
				offset.x += sc_NodeOffset;
			}
			while (nodes[childItem->GetID()].Depth < currentDepth)
			{
				offset.x -= sc_NodeOffset;
				currentDepth--;
			}
			childItem->Coords = offset;
			bool open = true;
			if (parent)
			{
				bUIHierarchyItem* parentItem = static_cast<bUIHierarchyItem*>(parent);
				open = parentItem->Open;
			}
			if (open)
				offset.y += tmpSize.y;
			else
				childItem->Open = false;

			currentDepth = nodes[childItem->ID].Depth;
			return false;
			});
	}

	
	bUITree::bUITree(const glm::vec2& coords, const glm::vec2& size, const glm::vec4& color, const std::string& label, const std::string& name, bUIElementType type)
		:
		bUIElement(coords, size, color,  label, name, type)		
	{
	}

	bUITree::bUITree(bUITree&& other) noexcept
		:
		bUIElement(std::move(other)),
		bUIHierarchyElement(std::move(other)),
		OnSelect(std::move(other.OnSelect))	
	{
	}

	void bUITree::PushQuads(bUIRenderer& renderer, uint32_t& scissorID)
	{
		ScissorID = scissorID;
		if (!Visible || !HandleVisibility(ScissorID))
			return;

		solveTreePosition(Size);
		renderer.Submit<bUITree>(*this, scissorID,
			bUI::GetConfig().GetSubTexture(bUIConfig::RightArrow),
			bUI::GetConfig().GetSubTexture(bUIConfig::DownArrow)
		);
	}

	bool bUITree::OnMouseMoved(const glm::vec2& mousePosition)
	{
		if (!Visible || !HandleVisibility(ScissorID))
			return false;

		glm::vec2 absolutePosition = GetAbsolutePosition();
		Hierarchy.Traverse([&](void* parent, void* child) ->bool {
			if (parent)
			{
				if (!static_cast<bUIHierarchyItem*>(parent)->Open)
					return false;
			}
			bUIHierarchyItem* childItem = static_cast<bUIHierarchyItem*>(child);
			childItem->Color = Color;
			glm::vec2 textSize = bUIHelper::FindTextSize(childItem->Label.c_str(), bUI::GetConfig().GetFont());
			glm::vec2 size = { Size.x + textSize.x, std::max(Size.y, textSize.y) };
			glm::vec2 itemAbsolutePosition = absolutePosition + childItem->GetCoords();
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
		if (!Visible || !HandleVisibility(ScissorID))
			return false;

		glm::vec2 absolutePosition = GetAbsolutePosition();
		bool result = false;
		Hierarchy.Traverse([&](void* parent, void* child) ->bool {
			if (parent)
			{
				if (!static_cast<bUIHierarchyItem*>(parent)->Open)
					return false;
			}
			bUIHierarchyItem* childItem = static_cast<bUIHierarchyItem*>(child);

			glm::vec2 itemAbsolutePosition = absolutePosition + childItem->GetCoords();
			glm::vec2 textPosition = itemAbsolutePosition + glm::vec2(Size.x, 0.0f);
			glm::vec2 textSize = bUIHelper::FindTextSize(childItem->Label.c_str(), bUI::GetConfig().GetFont());
			textSize.y = std::max(Size.y, textSize.y);

			if (Helper::Collide(itemAbsolutePosition, Size, mousePosition))
			{
				childItem->Open = !childItem->Open;
				result = true;
			}
			else if (Helper::Collide(textPosition, textSize, mousePosition))
			{
				if (OnSelect)
					OnSelect(childItem->GetKey());
				result = true;
			}
			return result;
		});
		return result;
	}

	bool bUITree::OnRightMousePressed(const glm::vec2& mousePosition)
	{
		if (!Visible || !HandleVisibility(ScissorID))
			return false;

		bool result = false;
		glm::vec2 absolutePosition = GetAbsolutePosition();
		Hierarchy.Traverse([&](void* parent, void* child) ->bool {
			if (parent)
			{
				if (!static_cast<bUIHierarchyItem*>(parent)->Open)
					return false;
			}
			bUIHierarchyItem* childItem = static_cast<bUIHierarchyItem*>(child);

			glm::vec2 textSize = bUIHelper::FindTextSize(childItem->Label.c_str(), bUI::GetConfig().GetFont());
			glm::vec2 size = { Size.x + textSize.x, std::max(Size.y, textSize.y) };
			glm::vec2 itemAbsolutePosition = absolutePosition + childItem->GetCoords();
			if (Helper::Collide(itemAbsolutePosition, size, mousePosition))
			{
				result = true;
				return true;
			}
			return false;
		});
		return result;
	}

	bUIDropdown::bUIDropdown(const glm::vec2& coords, const glm::vec2& size, const glm::vec4& color, const std::string& label, const std::string& name, bUIElementType type)
		:
		bUIElement(coords, size, color,  label, name, type)
	{
	}
	bUIDropdown::bUIDropdown(bUIDropdown&& other) noexcept
		:
		bUIElement(std::move(other)),
		bUIHierarchyElement(std::move(other))
	{
	}
	void bUIDropdown::PushQuads(bUIRenderer& renderer, uint32_t& scissorID)
	{
		ScissorID = scissorID;
		if (!Visible || !HandleVisibility(ScissorID))
			return;

		solveTreePosition(Size);
		renderer.Submit<bUIDropdown>(*this, scissorID,
			bUI::GetConfig().GetSubTexture(bUIConfig::Button),
			bUI::GetConfig().GetSubTexture(bUIConfig::RightArrow)
		);
	}
	bool bUIDropdown::OnMouseMoved(const glm::vec2& mousePosition)
	{
		if (!Visible || !HandleVisibility(ScissorID))
			return false;

		glm::vec2 absolutePosition = GetAbsolutePosition() + glm::vec2(0.0f, Size.y);
		Hierarchy.Traverse([&](void* parent, void* child) ->bool {
			if (parent)
			{
				if (!static_cast<bUIHierarchyItem*>(parent)->Open)
					return false;
			}
			bUIHierarchyItem* childItem = static_cast<bUIHierarchyItem*>(child);
			childItem->Color = Color;
			glm::vec2 textSize = bUIHelper::FindTextSize(childItem->Label.c_str(), bUI::GetConfig().GetFont());
			glm::vec2 size = { Size.x + textSize.x, std::max(Size.y, textSize.y) };
			glm::vec2 itemAbsolutePosition = absolutePosition + childItem->GetCoords();
			if (Helper::Collide(itemAbsolutePosition, size, mousePosition))
			{
				childItem->Color = bUI::GetConfig().GetColor(bUIConfig::HighlightColor);
			}
			return false;
			});
		return false;
	}
	bool bUIDropdown::OnLeftMousePressed(const glm::vec2& mousePosition)
	{
		if (!Visible || !HandleVisibility(ScissorID))
			return false;
		if (Helper::Collide(GetAbsolutePosition(), Size, mousePosition))
		{
			ChildrenVisible = !ChildrenVisible;
			for (auto& callback : Callbacks)
				callback(bUICallbackType::Active, *this);
			return BlockEvents;
		}

		bool result = false;
		glm::vec2 absolutePosition = GetAbsolutePosition() + glm::vec2(0.0f, Size.y);
		Hierarchy.Traverse([&](void* parent, void* child) ->bool {
			if (parent)
			{
				if (!static_cast<bUIHierarchyItem*>(parent)->Open)
					return false;
			}
			bUIHierarchyItem* childItem = static_cast<bUIHierarchyItem*>(child);
			glm::vec2 itemAbsolutePosition = absolutePosition + childItem->GetCoords();
			if (Helper::Collide(itemAbsolutePosition, Size, mousePosition))
			{
				if (OnSelect)
					OnSelect(childItem->GetKey());
				result = true;
			}
			return false;
		});
		return result;
	}
	bool bUIDropdown::OnRightMousePressed(const glm::vec2& mousePosition)
	{
		return false;
	}

	bUIInt::bUIInt(const glm::vec2& coords, const glm::vec2& size, const glm::vec4& color, const std::string& label, const std::string& name, bUIElementType type)
		:
		bUIElement(coords, size, color,  label, name, type)
	{
		SetValue(0);
	}

	void bUIInt::PushQuads(bUIRenderer& renderer, uint32_t& scissorID)
	{
		ScissorID = scissorID;
		if (!Visible || !HandleVisibility(ScissorID))
			return;
		renderer.Submit<bUIInt>(*this, scissorID, bUI::GetContext().Config.GetSubTexture(bUIConfig::Button));
	}

	bool bUIInt::OnMouseMoved(const glm::vec2& mousePosition)
	{
		return false;
	}

	bool bUIInt::OnLeftMousePressed(const glm::vec2& mousePosition)
	{
		if (!Visible || !HandleVisibility(ScissorID))
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
			return BlockEvents;
		}
		return false;
	}

	bool bUIInt::OnKeyPressed(int32_t mode, int32_t key)
	{
		if (Listen && Visible && HandleVisibility(ScissorID))
		{
			if (key == ToUnderlying(KeyCode::KEY_BACKSPACE))
			{
				if (InsertionIndex > 0)
					InsertionIndex--;
				Buffer[InsertionIndex] = '\0';
				for (auto& callback : Callbacks)
					callback(bUICallbackType::Active, *this);

				if (FitText)
					Size = bUIHelper::FindTextSize(Buffer, bUI::GetConfig().GetFont()) + (Borders * 2.0f);
				return BlockEvents;
			}
		}
		return false;
	}

	bool bUIInt::OnKeyTyped(char character)
	{
		if (Listen && Visible && HandleVisibility(ScissorID))
		{
			if (character >= toascii('0') && character <= toascii('9') 
			|| (character == toascii('-') && InsertionIndex == 0))
			{
				Buffer[InsertionIndex++] = character;
				for (auto& callback : Callbacks)
					callback(bUICallbackType::Active, *this);


				if (FitText)
					Size = bUIHelper::FindTextSize(Buffer, bUI::GetConfig().GetFont()) + (Borders * 2.0f);

				return BlockEvents;
			}
		}
		return false;
	}

	void bUIInt::SetValue(int32_t val)
	{
		if (!Listen)
		{
			memset(Buffer, 0, BufferSize);
			snprintf(Buffer, sizeof(Buffer), "%d", val);
			InsertionIndex = 0;
			while (Buffer[InsertionIndex] != '\0')
				InsertionIndex++;
		}
	}

	int32_t bUIInt::GetValue() const
	{
		return atoi(Buffer);
	}

	bUIFloat::bUIFloat(const glm::vec2& coords, const glm::vec2& size, const glm::vec4& color, const std::string& label, const std::string& name, bUIElementType type)
		:
		bUIElement(coords, size, color,  label, name, type)
	{
		SetValue(0.0f);
	}

	void bUIFloat::PushQuads(bUIRenderer& renderer, uint32_t& scissorID)
	{
		ScissorID = scissorID;
		if (!Visible || !HandleVisibility(ScissorID))
			return;
		renderer.Submit<bUIFloat>(*this, scissorID, bUI::GetContext().Config.GetSubTexture(bUIConfig::Button));
	}

	bool bUIFloat::OnMouseMoved(const glm::vec2& mousePosition)
	{
		return false;
	}

	bool bUIFloat::OnLeftMousePressed(const glm::vec2& mousePosition)
	{
		if (!Visible || !HandleVisibility(ScissorID))
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
			return BlockEvents;
		}
		return false;
	}

	bool bUIFloat::OnKeyPressed(int32_t mode, int32_t key)
	{
		if (Listen && Visible && HandleVisibility(ScissorID))
		{
			if (key == ToUnderlying(KeyCode::KEY_BACKSPACE))
			{
				if (InsertionIndex > 0)
					InsertionIndex--;
				Buffer[InsertionIndex] = '\0';
				for (auto& callback : Callbacks)
					callback(bUICallbackType::Active, *this);

				if (FitText)
					Size = bUIHelper::FindTextSize(Buffer, bUI::GetConfig().GetFont()) + (Borders * 2.0f);
				return BlockEvents;
			}
		}
		return false;
	}

	bool bUIFloat::OnKeyTyped(char character)
	{
		if (Listen && Visible && HandleVisibility(ScissorID))
		{
			if (character >= toascii('0') && character <= toascii('9') 
				|| (character == toascii('-') && InsertionIndex == 0)
				|| character == toascii('.'))
			{
				Buffer[InsertionIndex++] = character;
				for (auto& callback : Callbacks)
					callback(bUICallbackType::Active, *this);

				
				if (FitText)
					Size = bUIHelper::FindTextSize(Buffer, bUI::GetConfig().GetFont()) + (Borders * 2.0f);

				return BlockEvents;
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
		return (float)atof(Buffer);
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

	bUIString::bUIString(const glm::vec2& coords, const glm::vec2& size, const glm::vec4& color, const std::string& label, const std::string& name, bUIElementType type)
		:
		bUIElement(coords, size, color,  label, name, type)
	{
		SetValue("");
	}

	void bUIString::PushQuads(bUIRenderer& renderer, uint32_t& scissorID)
	{
		ScissorID = scissorID;
		if (!Visible || !HandleVisibility(ScissorID))
			return;
		renderer.Submit<bUIString>(*this, scissorID, bUI::GetContext().Config.GetSubTexture(bUIConfig::Button));
	}

	bool bUIString::OnMouseMoved(const glm::vec2& mousePosition)
	{
		return false;
	}

	bool bUIString::OnLeftMousePressed(const glm::vec2& mousePosition)
	{
		if (!Visible || !HandleVisibility(ScissorID))
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
			return BlockEvents;
		}
		return false;
	}

	bool bUIString::OnKeyPressed(int32_t mode, int32_t key)
	{
		if (Listen && Visible && HandleVisibility(ScissorID))
		{
			if (key == ToUnderlying(KeyCode::KEY_BACKSPACE))
			{
				if (InsertionIndex > 0)
					InsertionIndex--;
				Buffer.erase(Buffer.begin() + InsertionIndex);
				for (auto& callback : Callbacks)
					callback(bUICallbackType::Active, *this);

				if (FitText)
					Size = bUIHelper::FindTextSize(Buffer.c_str(), bUI::GetConfig().GetFont()) + (Borders * 2.0f);
				return BlockEvents;
			}
			else if (key == ToUnderlying(KeyCode::KEY_ENTER))
			{
				Buffer.push_back('\n');
				InsertionIndex++;
				if (FitText)
					Size = bUIHelper::FindTextSize(Buffer.c_str(), bUI::GetConfig().GetFont()) + (Borders * 2.0f);
				return BlockEvents;
			}
		}
		return false;
	}

	bool bUIString::OnKeyTyped(char character)
	{
		if (Listen && Visible && HandleVisibility(ScissorID))
		{		
			Buffer.push_back(character);
			InsertionIndex++;
			for (auto& callback : Callbacks)
				callback(bUICallbackType::Active, *this);

			if (FitText)
				Size = bUIHelper::FindTextSize(Buffer.c_str(), bUI::GetConfig().GetFont()) + (Borders * 2.0f);

			return BlockEvents;
		}
		return false;
	}

	void bUIString::SetValue(const std::string& value)
	{
		Buffer = value;
		InsertionIndex = Buffer.size();
	}

	bUIImage::bUIImage(const glm::vec2& coords, const glm::vec2& size, const glm::vec4& color, const std::string& label, const std::string& name, bUIElementType type)
		:
		bUIElement(coords, size, color,  label, name, type)
	{
	}

	void bUIImage::PushQuads(bUIRenderer& renderer, uint32_t& scissorID)
	{
		ScissorID = scissorID;
		if (!Visible || !HandleVisibility(ScissorID))
			return;
		renderer.Submit<bUIImage>(*this, scissorID);
	}
	void bUIImage::OnUpdate()
	{
		if (FitParent && Parent)
		{
			Coords = glm::vec2(0.0f);
			Size = Parent->Size;
		}
	}
	bool bUIImage::OnMouseMoved(const glm::vec2& mousePosition)
	{
		if (!Visible || !HandleVisibility(ScissorID))
			return false;
		if (Helper::Collide(GetAbsolutePosition(), Size, mousePosition))
		{
			if (EnableHighlight)
				ActiveColor = bUI::GetConfig().GetColor(bUIConfig::HighlightColor);
			for (auto& callback : Callbacks)
				callback(bUICallbackType::Hoover, *this);
			return BlockEvents;
		}
		ActiveColor = Color;
		return false;
	}
	bUIText::bUIText(const glm::vec2& coords, const glm::vec2& size, const glm::vec4& color, const std::string& label, const std::string& name, bUIElementType type)
		:
		bUIElement(coords, size, color,  label, name, type)
	{
	}
	void bUIText::PushQuads(bUIRenderer& renderer, uint32_t& scissorID)
	{
		ScissorID = scissorID;
		if (!Visible || !HandleVisibility(ScissorID))
			return;
		renderer.Submit<bUIText>(*this, scissorID);
	}
	glm::vec2 bUIText::GetSize() const
	{
		return bUIHelper::FindTextSize(Label.c_str(), bUI::GetConfig().GetFont());
	}
	

	
}