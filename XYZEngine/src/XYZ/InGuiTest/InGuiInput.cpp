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
	bool IGInput::OnMouseButtonPress(MouseButtonPressEvent& e, IGContext& context)
	{
		if (e.IsButtonPressed(MouseCode::MOUSE_BUTTON_LEFT))
		{
			auto [mx, my] = Input::GetMousePosition();
			glm::vec2 mousePosition = { mx ,my };
			for (auto& pool : context.Allocator.GetPools())
			{
				for (int32_t id : pool.GetParentIDs())
				{
					IGElement* parent = static_cast<IGElement*>(pool.GetHierarchy().GetData(id));
					if (Helper::Collide(parent->Position, parent->Size, mousePosition) && parent->Active)
					{
						context.RenderData.Rebuild = true;
						context.FrameData.MouseOffset = mousePosition - parent->Position;
						pool.GetHierarchy().TraverseNodeChildren(id, [&](void* parent, void* child) -> bool {

							IGElement* childElement = static_cast<IGElement*>(child);
							if (!childElement->Active)
								return false;

							if (childElement->OnLeftClick(mousePosition))
							{
								e.Handled = true;
								return true;
							}
						});
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
	bool IGInput::OnMouseMove(MouseMovedEvent& e, IGContext& context)
	{
		auto [mx, my] = Input::GetMousePosition();
		glm::vec2 mousePosition = { mx , my };

		for (auto& pool : context.Allocator.GetPools())
		{
			for (int32_t id : pool.GetParentIDs())
			{
				IGElement* parent = static_cast<IGElement*>(pool.GetHierarchy().GetData(id));
				if (Helper::Collide(parent->Position, parent->Size, mousePosition) && parent->Active)
				{
					context.RenderData.Rebuild = true;
					pool.GetHierarchy().TraverseNodeChildren(id, [&](void* parent, void* child) -> bool {

						IGElement* childElement = static_cast<IGElement*>(child);
						if (!childElement->Active)
							return false;

						childElement->OnMouseMove(mousePosition);
						return false;
						});
				}
				IGWindow* window = dynamic_cast<IGWindow*>(parent);
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