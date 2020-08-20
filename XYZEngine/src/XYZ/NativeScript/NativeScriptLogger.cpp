#include "stdafx.h"
#include "NativeScriptLogger.h"



#include <stdarg.h>
#include <assert.h>
#include <iostream>

#ifdef _WIN32
#include "Windows.h"
#pragma warning( disable : 4996 4800 )
#endif


namespace XYZ {
	void NativeScriptLogger::LogError(const char* format, ...)
	{
		va_list args;
		va_start(args, format);
		LogInternal(format, args);
	}
	void NativeScriptLogger::LogWarning(const char* format, ...)
	{
		va_list args;
		va_start(args, format);
		LogInternal(format, args);
	}
	void NativeScriptLogger::LogInfo(const char* format, ...)
	{
		va_list args;
		va_start(args, format);
		LogInternal(format, args);
	}
	void NativeScriptLogger::LogInternal(const char* format, va_list args)
	{
		int result = vsnprintf(m_buff, LOGSYSTEM_MAX_BUFFER - 1, format, args);
		// Make sure there's a limit to the amount of rubbish we can output
		m_buff[LOGSYSTEM_MAX_BUFFER - 1] = '\0';

		std::cout << m_buff;
#ifdef _WIN32
		OutputDebugStringA(m_buff);
#endif
	}
}