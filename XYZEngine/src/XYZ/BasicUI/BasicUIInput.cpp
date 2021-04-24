#include "stdafx.h"
#include "BasicUIInput.h"

#include "XYZ/Core/Input.h"


namespace XYZ {

	static bool OnLeftMouseButtonPressRecursive(bUIElement* element, bUIAllocator& allocator, const glm::vec2& mousePosition)
	{
		if (element->OnLeftMousePressed(mousePosition))
			return true;

		bool result = false;
		if (element->ChildrenVisible)
		{
			allocator.GetHierarchy().TraverseNodeChildren(element->GetID(), [&](void* parent, void* child) -> bool {

				bUIElement* childElement = static_cast<bUIElement*>(child);
				result = OnLeftMouseButtonPressRecursive(childElement, allocator,mousePosition);
				return result;
			});
		}
		return result;
	}

	static bool OnRightMouseButtonPressRecursive(bUIElement* element, bUIAllocator& allocator, bUIEditData& editData, const glm::vec2& mousePosition)
	{
		if (element->OnRightMousePressed(mousePosition))
		{
			editData.Element = element;
			editData.MouseOffset = mousePosition - element->Coords;
			return true;
		}
		bool result = false;
		if (!element->Locked && element->ChildrenVisible)
		{
			allocator.GetHierarchy().TraverseNodeChildren(element->GetID(), [&](void* parent, void* child) -> bool {

				bUIElement* childElement = static_cast<bUIElement*>(child);
				result = OnRightMouseButtonPressRecursive(childElement, allocator, editData, mousePosition);
				return result;
			});
		}
		return result;
	}

	static bool OnMouseScrollRecursive(bUIElement* element, bUIAllocator& allocator, const glm::vec2& mousePosition, const glm::vec2& offset)
	{		
		bool result = false;
		allocator.GetHierarchy().TraverseNodeChildren(element->GetID(), [&](void* parent, void* child) -> bool {

			bUIElement* childElement = static_cast<bUIElement*>(child);
			if (OnMouseScrollRecursive(childElement, allocator, mousePosition, offset))
				result = true;
			return result;
		});

		if (!result)
			result = element->OnMouseScrolled(mousePosition, offset);

		return result;
	}


	bool bUIInput::OnMouseButtonPress(MouseButtonPressEvent& event, bUIEditData& editData, bUIData& data)
	{
		auto [mx, my] = Input::GetMousePosition();
		glm::vec2 mousePosition = { mx, my };
		if (event.IsButtonPressed(MouseCode::MOUSE_BUTTON_LEFT))
		{
			bool result = false;
			for (bUIAllocator& allocator : data.m_Allocators)
			{
				int32_t root = allocator.GetHierarchy().GetRoot();
				allocator.GetHierarchy().TraverseNodeSiblings(root, [&](void* parent, void* child) -> bool {			
					bUIElement* childElement = static_cast<bUIElement*>(child);
					result = OnLeftMouseButtonPressRecursive(childElement, allocator, mousePosition);
					return result;
				});
			}
			return result;
		}
		else if (event.IsButtonPressed(MouseCode::MOUSE_BUTTON_RIGHT))
		{
			if (!bUI::GetConfig().IsLocked())
			{
				bool result = false;
				for (bUIAllocator& allocator : data.m_Allocators)
				{
					int32_t root = allocator.GetHierarchy().GetRoot();
					allocator.GetHierarchy().TraverseNodeSiblings(root, [&](void* parent, void* child) -> bool {			
						bUIElement* childElement = static_cast<bUIElement*>(child);
						result = OnRightMouseButtonPressRecursive(childElement, allocator, editData, mousePosition);
						return result;
					});
				}
				return result;
			}
		}
		return false;
	}
	bool bUIInput::OnMouseButtonRelease(MouseButtonReleaseEvent& event, bUIEditData& editData, bUIData& data)
	{
		if (event.IsButtonReleased(MouseCode::MOUSE_BUTTON_LEFT))
		{
			for (bUIAllocator& allocator : data.m_Allocators)
			{
				for (size_t i = 0; i < allocator.Size(); ++i)
				{
					bUIElement* element = allocator.GetElement<bUIElement>(i);
					if (element->OnLeftMouseReleased())
						return true;
				}
			}
		}
		else if (event.IsButtonReleased(MouseCode::MOUSE_BUTTON_RIGHT))
		{
			editData.Element = nullptr;
		}
		return false;
	}
	bool bUIInput::OnMouseMove(MouseMovedEvent& event, bUIEditData& editData, bUIData& data)
	{
		auto [mx, my] = Input::GetMousePosition();
		glm::vec2 mousePosition = { mx , my };
		if (editData.Element)
		{
			editData.Element->Coords = mousePosition - editData.MouseOffset;
		}
		for (bUIAllocator& allocator : data.m_Allocators)
		{
			for (size_t i = 0; i < allocator.Size(); ++i)
			{
				if (allocator.GetElement<bUIElement>(i)->OnMouseMoved(mousePosition))
					return true;
			}
		}
		return false;
	}
	bool bUIInput::OnMouseScroll(MouseScrollEvent& event, bUIEditData& editData, bUIData& data)
	{
		auto [mx, my] = Input::GetMousePosition();
		glm::vec2 mousePosition = { mx, my };
		glm::vec2 offset = { event.GetOffsetX(), event.GetOffsetY() };
		bool result = false;
		for (bUIAllocator& allocator : data.m_Allocators)
		{
			int32_t root = allocator.GetHierarchy().GetRoot();
			allocator.GetHierarchy().TraverseNodeSiblings(root, [&](void* parent, void* child) -> bool {			
				bUIElement* childElement = static_cast<bUIElement*>(child);
				if (OnMouseScrollRecursive(childElement, allocator, mousePosition, offset))
					result = true;
				return result;
			});
		}
		return result;
	}
	bool bUIInput::OnKeyType(KeyTypedEvent& event)
	{
		if (bUIListener::GetListener())
		{
			return bUIListener::GetListener()->OnKeyTyped(event.GetKey());
		}
		return false;
	}
	bool bUIInput::OnKeyPress(KeyPressedEvent& event)
	{
		if (bUIListener::GetListener())
		{
			return bUIListener::GetListener()->OnKeyPressed(event.GetMod(), event.GetKey());
		}
		return false;
	}
}