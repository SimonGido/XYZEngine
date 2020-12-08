#include "stdafx.h"

#include "XYZ/Core/Logger.h"

namespace XYZ {
	enum AnsiCode
	{
		COLOR_BLACK_F = 30,
		COLOR_RED_F = 31,
		COLOR_GREEN_F = 32,
		COLOR_YELLOW_F = 33,
		COLOR_BLUE_F = 34,
		COLOR_MAGENTA_F = 35,
		COLOR_CYAN_F = 36,
		COLOR_WHITE_F = 37,

		COLOR_BLACK_B = 40,
		COLOR_RED_B = 41,
		COLOR_GREEN_B = 42,
		COLOR_YELLOW_B = 43,
		COLOR_BLUE_B = 44,
		COLOR_MAGENTA_B = 45,
		COLOR_CYAN_B = 46,
		COLOR_WHITE_B = 47
	};

	Logger::Logger()
		:
		m_RedColor(COLOR_RED_F),
		m_YellowColor(COLOR_YELLOW_F),
		m_GreenColor(COLOR_GREEN_F),
		m_PurpleColor(COLOR_MAGENTA_F),
		m_WhiteColor(COLOR_WHITE_F),
		m_LogLevel(LogLevel::NOLOG)
	{
		m_FileName = "XYZLog.txt";
		m_LogFile.open(m_FileName.c_str(), std::ios::out | std::ios::app);
	}

	void Logger::SetColor(const int color)
	{
		printf("\033[1;%dm", color);
	}
}