#include "stdafx.h"
#include "IGInput.h"
#include "IGContext.h"
#include "IGUIElements.h"

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
	}

	static bool OnMouseButtonPressRecursive(IGElement* parentElement, IGContext& context, IGPool& pool, const glm::vec2& mousePosition, bool& handled)
	{
		bool result = false;
		if (parentElement->Active && parentElement->ActiveChildren)
		{
			pool.GetHierarchy().TraverseNodeChildren(parentElement->GetID(), [&](void* parent, void* child) -> bool {

				IGElement* childElement = static_cast<IGElement*>(child);
				if (childElement->Active && childElement->ListenToInput)
				{
					OnMouseButtonPressRecursive(childElement, context, pool, mousePosition, handled);	 
					childElement->OnLeftClick(mousePosition, handled);
				}
				return false;
			});
		}	
		return handled;
	}

	bool IGInput::OnMouseButtonPress(MouseButtonPressEvent& e, IGContext& context)
	{
		if (e.IsButtonPressed(MouseCode::MOUSE_BUTTON_LEFT))
		{
			auto [mx, my] = Input::GetMousePosition();
			glm::vec2 mousePosition = { mx ,my };
			if (context.Dockspace.OnMouseLeftPress(mousePosition, e.Handled))
				return true;
			for (auto& pool : context.Allocator.GetPools())
			{
				for (auto id : pool.GetRootElementIDs())
				{
					IGWindow* parentElement = static_cast<IGWindow*>(pool.GetHierarchy().GetData(id));			
					bool result = OnMouseButtonPressRecursive(parentElement, context, pool, mousePosition, e.Handled);
					if (result || parentElement->OnLeftClick(mousePosition, e.Handled))
					{
						if (IS_SET(parentElement->Flags, IGWindow::Moved))
						{
							context.FrameData.MouseOffset = mousePosition - parentElement->Position;
							context.Dockspace.RemoveWindow(parentElement);
						}
						return true;
					}
				}
			}
		}
		return false;
	}
	bool IGInput::OnMouseButtonRelease(MouseButtonReleaseEvent& e, IGContext& context)
	{
		if (e.IsButtonReleased(MouseCode::MOUSE_BUTTON_LEFT))
		{
			auto [mx, my] = Input::GetMousePosition();
			glm::vec2 mousePosition = { mx , my };
			context.Dockspace.OnMouseLeftRelease();
			context.Dockspace.SetVisibility(false);

			for (auto& pool : context.Allocator.GetPools())
			{
				for (auto id : pool.GetRootElementIDs())
				{
					IGWindow* window = static_cast<IGWindow*>(pool.GetHierarchy().GetData(id));
					if (IS_SET(window->Flags, IGWindow::Moved))
					{
						if (context.Dockspace.InsertWindow(window, mousePosition))
							window->Flags |= IGWindow::Docked;
					}
				}
				for (size_t i = 0; i < pool.Size(); ++i)
				{
					IGElement* element = pool[i];
					if (!element->Active)
						continue;

					element->OnLeftRelease(mousePosition, e.Handled);
				}
			}
		}
		return false;
	}

	static bool OnMouseMoveRecursive(IGElement* parentElement, IGContext& context, IGPool& pool, const glm::vec2& mousePosition, bool& handled)
	{
		if (parentElement->Active && parentElement->ActiveChildren)
		{
			context.RenderData.Rebuild = true;
			pool.GetHierarchy().TraverseNodeChildren(parentElement->GetID(), [&](void* parent, void* child) -> bool {

				IGElement* childElement = static_cast<IGElement*>(child);
				if (childElement->Active && childElement->ListenToInput)
				{
					childElement->OnMouseMove(mousePosition, handled);
					OnMouseMoveRecursive(childElement, context, pool, mousePosition, handled);
				}
				return false;
			});
		}	
		return false;
	}

	bool IGInput::OnMouseMove(MouseMovedEvent& e, IGContext& context)
	{
		auto [mx, my] = Input::GetMousePosition();
		glm::vec2 mousePosition = { mx , my };

		context.Dockspace.OnMouseMove(mousePosition, e.Handled);

		for (auto& pool : context.Allocator.GetPools())
		{
			for (auto id : pool.GetRootElementIDs())
			{
				IGElement* parentElement = static_cast<IGElement*>(pool.GetHierarchy().GetData(id));
				bool result = OnMouseMoveRecursive(parentElement, context, pool, mousePosition, e.Handled);
				parentElement->OnMouseMove(mousePosition, e.Handled);
			
				IGWindow* window = dynamic_cast<IGWindow*>(parentElement);
				if (window)
				{
					glm::vec2 mouseDiff = mousePosition - context.FrameData.MouseOffset;
					window->HandleActions(mousePosition, mouseDiff, e.Handled);
					if (IS_SET(window->Flags, IGWindow::Moved))
						context.Dockspace.SetVisibility(true);
				}
			}
		}
		return false;
	}
	bool IGInput::OnMouseScroll(MouseScrollEvent& e, IGContext& context)
	{
		auto [mx, my] = Input::GetMousePosition();
		glm::vec2 mousePosition = { mx , my };
		for (auto& pool : context.Allocator.GetPools())
		{
			pool.GetHierarchy().Traverse([&](void* parent, void* child) -> bool {

				IGElement* childElement = static_cast<IGElement*>(child);
				if (childElement->OnMouseScroll(mousePosition, e.GetOffsetY(), e.Handled))
					return true;

				return false;
			});
		}
		return e.Handled;
	}

	bool IGInput::OnKeyType(KeyTypedEvent& e, IGContext& context)
	{
		for (auto& pool : context.Allocator.GetPools())
		{
			pool.GetHierarchy().Traverse([&](void* parent, void* child) -> bool {

				IGElement* childElement = static_cast<IGElement*>(child);
				if (childElement->OnKeyType((char)e.GetKey(), e.Handled))
					return true;
				
				return false;
			});
		}
		return e.Handled;
	}
	bool IGInput::OnKeyPress(KeyPressedEvent& e, IGContext& context)
	{
		for (auto& pool : context.Allocator.GetPools())
		{
			pool.GetHierarchy().Traverse([&](void* parent, void* child) -> bool {

				IGElement* childElement = static_cast<IGElement*>(child);
				if (childElement->OnKeyPress(e.GetMod(), e.GetKey(), e.Handled))
					return true;

				return false;
			});
		}
		return e.Handled;
	}
}