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

	static bool OnMouseButtonPressRecursive(IGElement* parentElement, IGContext& context, IGPool& pool, const glm::vec2& mousePosition)
	{
		bool result = false;
		if (parentElement->Active && parentElement->ActiveChildren)
		{
			pool.GetHierarchy().TraverseNodeChildren(parentElement->GetID(), [&](void* parent, void* child) -> bool {

				IGElement* childElement = static_cast<IGElement*>(child);
				if (childElement->Active && childElement->ListenToInput)
				{
					bool tmp  = OnMouseButtonPressRecursive(childElement, context, pool, mousePosition);	 
					if (!result)
						result = tmp;
					if (!result && childElement->OnLeftClick(mousePosition))
						result = true;
				}
				return false;
			});
		}	
		return result;
	}

	bool IGInput::OnMouseButtonPress(MouseButtonPressEvent& e, IGContext& context)
	{
		if (e.IsButtonPressed(MouseCode::MOUSE_BUTTON_LEFT))
		{
			auto [mx, my] = Input::GetMousePosition();
			glm::vec2 mousePosition = { mx ,my };
			for (auto& pool : context.Allocator.GetPools())
			{
				for (auto id : pool.GetRootElementIDs())
				{
					IGElement* parentElement = static_cast<IGElement*>(pool.GetHierarchy().GetData(id));			
					bool result = OnMouseButtonPressRecursive(parentElement, context, pool, mousePosition);
					if (result || parentElement->OnLeftClick(mousePosition))
					{
						context.FrameData.MouseOffset = mousePosition - parentElement->Position;
						e.Handled = true;
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
			for (auto& pool : context.Allocator.GetPools())
			{
				for (size_t i = 0; i < pool.Size(); ++i)
				{
					IGElement* element = pool[i];
					if (!element->Active)
						continue;

					element->OnLeftRelease(mousePosition);
				}
			}
		}
		return false;
	}

	static bool OnMouseMoveRecursive(IGElement* parentElement, IGContext& context, IGPool& pool, const glm::vec2& mousePosition)
	{
		if (parentElement->Active && parentElement->ActiveChildren && Helper::Collide(parentElement->Position, parentElement->Size, mousePosition))
		{
			context.RenderData.Rebuild = true;
			pool.GetHierarchy().TraverseNodeChildren(parentElement->GetID(), [&](void* parent, void* child) -> bool {

				IGElement* childElement = static_cast<IGElement*>(child);
				if (childElement->Active && childElement->ListenToInput)
				{
					childElement->OnMouseMove(mousePosition);
					OnMouseMoveRecursive(childElement, context, pool, mousePosition);
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
	
		for (auto& pool : context.Allocator.GetPools())
		{
			for (auto id : pool.GetRootElementIDs())
			{
				IGElement* parentElement = static_cast<IGElement*>(pool.GetHierarchy().GetData(id));
				bool result = OnMouseMoveRecursive(parentElement, context, pool, mousePosition);
				if (result || parentElement->OnMouseMove(mousePosition))
					e.Handled = true;
				
				IGWindow* window = dynamic_cast<IGWindow*>(parentElement);
				if (window)
				{
					if (IS_SET(window->Flags, IGWindow::Moved))
					{
						window->Position = mousePosition - context.FrameData.MouseOffset;
						e.Handled = true;
					}
					else if (IS_SET(window->Flags, IGWindow::LeftResize))
					{
						window->Size.x = window->GetAbsolutePosition().x + window->Size.x - mousePosition.x;
						window->Position.x = mousePosition.x;
						e.Handled = true;
					}
					else if (IS_SET(window->Flags, IGWindow::RightResize))
					{
						window->Size.x = mousePosition.x - window->GetAbsolutePosition().x;
						e.Handled = true;
					}

					if (IS_SET(window->Flags, IGWindow::BottomResize))
					{
						window->Size.y = mousePosition.y - window->GetAbsolutePosition().y;
						e.Handled = true;
					}
				}
			}
		}
		return false;
	}
	bool IGInput::OnMouseScroll(MouseScrollEvent& e, IGContext& context)
	{
		return false;
	}

	bool IGInput::OnKeyType(KeyTypedEvent& e, IGContext& context)
	{
		for (auto& pool : context.Allocator.GetPools())
		{
			pool.GetHierarchy().Traverse([&](void* parent, void* child) -> bool {

				IGElement* childElement = static_cast<IGElement*>(child);
				if (childElement->OnKeyType((char)e.GetKey()))
				{
					e.Handled = true;
					return true;
				}
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
				if (childElement->OnKeyPress(e.GetMod(), e.GetKey()))
				{
					e.Handled = true;
					return true;
				}
				return false;
			});
		}
		return e.Handled;
	}
}