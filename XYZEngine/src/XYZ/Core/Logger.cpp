#include "stdafx.h"
#include "Logger.h"


#include <time.h>


namespace XYZ {
	Logger Logger::s_Instance;

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


