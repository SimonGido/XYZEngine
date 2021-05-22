#include "stdafx.h"
#include "InGuiFrame.h"


namespace XYZ {
	InGuiFrame::InGuiFrame()
		:
		CurrentWindow(nullptr),
		MovedWindowOffset(0.0f),
		CurrentMenuWidth(0.0f)
	{
		XYZ_ASSERT(WindowQueue.empty(), "Window queue is not empty, forgot end?");
	}
}