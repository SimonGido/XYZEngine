#include "stdafx.h"
#include "InGuiFrame.h"


namespace XYZ {
	InGuiFrame::InGuiFrame()
		:
		CurrentWindow(nullptr),
		MovedWindowOffset(0.0f),
		CurrentMenuWidth(0.0f)
	{
		
	}
	InGuiFrame::~InGuiFrame()
	{
		XYZ_ASSERT(WindowQueue.empty(), "Window queue is not empty, forgot end?");
	}
}