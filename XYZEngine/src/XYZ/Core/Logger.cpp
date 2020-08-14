#include "stdafx.h"
#include "Logger.h"
#include "Platform/Windows/WindowsLogger.h"
#include "Platform/Linux/LinuxLogger.h"

#include <time.h>


namespace XYZ {
	std::unique_ptr<Logger> Logger::s_Instance;

	void Logger::Init()
	{
		s_Instance = Logger::Create();
	}

	std::unique_ptr<Logger> Logger::Create()
	{
		#ifdef XYZ_PLATFORM_WINDOWS
			return std::make_unique<WindowsLogger>();
		#endif // XYZ_PLATFORM_WINDOWS
			return std::make_unique<LinuxLogger>();
	}

	Logger::Logger(int redColor,
		int yellowColor,
		int greenColor,
		int purpleColor,
		int whiteColor)
		:
		m_RedColor(redColor),
		m_YellowColor(yellowColor),
		m_GreenColor(greenColor),
		m_PurpleColor(purpleColor),
		m_WhiteColor(whiteColor)
	{
		m_LogLevel = TRACE;
		m_FileName = "XYZLog.txt";
		m_LogFile.open(m_FileName.c_str(), std::ios::out | std::ios::app);
	}

	std::string Logger::currentDateTime()
	{
		time_t     now = time(NULL);
		struct tm  tstruct;
		char       buf[80];
		localtime_s(&tstruct, &now);
		strftime(buf, sizeof(buf), "%Y-%m-%d %X", &tstruct);
		return buf;
	}

}


