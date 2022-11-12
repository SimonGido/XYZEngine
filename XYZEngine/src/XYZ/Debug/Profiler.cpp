#include "stdafx.h"
#include "Profiler.h"

#include <optick.h>

namespace XYZ {
	void Profiler::Frame(const char* name)
	{
		OPTICK_FRAME(name);
	}
	void Profiler::Event(const char* functionName)
	{
		OPTICK_EVENT(functionName);
	}
	void Profiler::EventDynamic(const char* name)
	{
		OPTICK_EVENT_DYNAMIC(name);
	}
	void Profiler::Thread(const char* name)
	{
		OPTICK_THREAD(name);
	}
	void Profiler::Shutdown()
	{
		OPTICK_SHUTDOWN();
	}
}