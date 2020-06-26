#pragma once
#include "XYZ/Core/Input.h"

namespace XYZ {
/**	@class WindowsInput
 *	Enables player input on Windows
 *
 */
	class WindowsInput : public Input
	{
	protected:
		virtual bool IsKeyPressedImpl(int keycode) override;
		virtual bool IsMouseButtonPressedImpl(int button) override;
		virtual std::pair<float, float> GetMousePositionImpl() override;
		virtual float GetMouseXImpl() override;
		virtual float GetMouseYImpl() override;
		virtual std::pair<int, int> GetWindowSizeImpl() override;
	};
}