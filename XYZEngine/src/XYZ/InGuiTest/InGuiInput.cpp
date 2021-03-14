#include "stdafx.h"
#include "InGuiInput.h"
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

	bool IGInput::OnMouseButtonPress(MouseButtonPressEvent& e, IGAllocator& allocator)
	{
		if (e.IsButtonPressed(MouseCode::MOUSE_BUTTON_LEFT))
		{
			auto [mx, my] = Input::GetMousePosition();
			for (auto& pool : allocator.GetPools())
			{
				for (size_t i = 0; i < pool.Size(); ++i)
				{
					IGElement* element = pool[i];
					element->ReturnType = IGReturnType::None;
					if (element->OnLeftClick({ mx, my }))
						e.Handled = true;
				}
			}
		}
		return false;
	}
	bool IGInput::OnMouseButtonRelease(MouseButtonReleaseEvent& e, IGAllocator& allocator)
	{
		return false;
	}
	bool IGInput::OnMouseMove(MouseMovedEvent& e, IGAllocator& allocator)
	{
		return false;
	}
	bool IGInput::OnMouseScroll(MouseScrollEvent& e, IGAllocator& allocator)
	{
		return false;
	}
}