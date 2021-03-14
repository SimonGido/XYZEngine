#include "stdafx.h"
#include "InGuiInput.h"
#include "InGuiContext.h"

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
				for (size_t i = 0; i < pool.Size(); ++i)
				{
					IGElement* element = pool[i];
					if (!element->Active)
						continue;

					element->ReturnType = IGReturnType::None;
					if (!e.Handled && element->OnLeftClick(mousePosition))
					{
						e.Handled = true;
						context.FrameData.MouseOffset = mousePosition - element->Position;
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

					element->ReturnType = IGReturnType::None;
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
			for (size_t i = 0; i < pool.Size(); ++i)
			{
				IGElement* element = pool[i];
				if (!element->Active)
					continue;

				element->ReturnType = IGReturnType::None;
				if (!e.Handled && element->OnMouseMove(mousePosition))
					e.Handled = true;
				if (auto window = dynamic_cast<IGWindow*>(element))
				{
					if (IS_SET(window->Flags, IGWindow::Moved))
						window->Position = mousePosition - context.FrameData.MouseOffset;
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