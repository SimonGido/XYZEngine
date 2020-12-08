#include "stdafx.h"
#include "XYZ/Core/Logger.h"

#include <Windows.h>

namespace XYZ {

	static void* s_Handle = nullptr;

	enum WindowsColor
	{
		COLOR_BLUE_F = 1,
		COLOR_GREEN_F = 2,
		COLOR_RED_F = 4,
		COLOR_PURPLE_F = 5,
		COLOR_YELLOW_F = 6,
		COLOR_WHITE_F = 15,
	};
	Logger::Logger()
		:
		m_RedColor(COLOR_RED_F),
		m_YellowColor(COLOR_YELLOW_F),
		m_GreenColor(COLOR_GREEN_F),
		m_PurpleColor(COLOR_PURPLE_F),
		m_WhiteColor(COLOR_WHITE_F),
		m_LogLevel(LogLevel::NOLOG)
	{
		s_Handle = GetStdHandle(STD_OUTPUT_HANDLE);

		m_FileName = "XYZLog.txt";
		m_LogFile.open(m_FileName.c_str(), std::ios::out | std::ios::app);
	}

	void Logger::SetColor(const int color)
	{
		SetConsoleTextAttribute(s_Handle, (WORD)color);
	}

}