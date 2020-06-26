#include "stdafx.h"
#include "LinuxLogger.h"

namespace XYZ {
	LinuxLogger::LinuxLogger()
		:
		Logger(COLOR_RED_F, COLOR_YELLOW_F, COLOR_GREEN_F, COLOR_MAGENTA_F, COLOR_WHITE_F)
	{
	}
	void LinuxLogger::SetColor(const int color)
	{
		printf("\033[1;%dm", color);
	}
}