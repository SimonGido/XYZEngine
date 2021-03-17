#include "stdafx.h"
#include "InGuiInput.h"
#include "InGuiContext.h"
#include "InGuiUIElements.h"

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
		if (parentElement->Active && parentElement->ActiveChildren && Helper::Collide(parentElement->Position, parentElement->Size, mousePosition))
		{
			context.RenderData.Rebuild = true;
			context.FrameData.MouseOffset = mousePosition - parentElement->Position;
			pool.GetHierarchy().TraverseNodeChildren(parentElement->GetID(), [&](void* parent, void* child) -> bool {

				IGElement* childElement = static_cast<IGElement*>(child);
				if (childElement->Active)
				{
					result = OnMouseButtonPressRecursive(childElement, context, pool, mousePosition);
					if (!result && childElement->OnLeftClick(mousePosition))
					{
						result = true;
						return true;
					}
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
					if (!result && parentElement->OnLeftClick(mousePosition))
					{
						e.Handled = true;
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
				if (childElement->Active)
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
				if (!result && parentElement->OnMouseMove(mousePosition))
					e.Handled = true;
				
				IGWindow* window = dynamic_cast<IGWindow*>(parentElement);
				if (window && IS_SET(window->Flags, IGWindow::Moved))
				{
					window->Position = mousePosition - context.FrameData.MouseOffset;
					context.RenderData.Rebuild = true;
				}
			}
		}
		return false;
	}
	bool IGInput::OnMouseScroll(MouseScrollEvent& e, IGContext& context)
	{
		return false;
	}
}