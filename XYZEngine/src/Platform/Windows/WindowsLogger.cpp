#include "stdafx.h"
#include "WindowsLogger.h"

#include <Windows.h>

namespace XYZ {
	WindowsLogger::WindowsLogger()
		: Logger(COLOR_RED_F,COLOR_YELLOW_F,COLOR_GREEN_F,COLOR_PURPLE_F,COLOR_WHITE_F)
	{
		m_Handle = GetStdHandle(STD_OUTPUT_HANDLE);
	}
	void WindowsLogger::SetColor(const int color)
	{
		SetConsoleTextAttribute(m_Handle, (WORD)color);
	}
}