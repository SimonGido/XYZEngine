#pragma once
#include "XYZ/Core/Core.h"



namespace XYZ {
	class XYZ_API Profiler
	{
	public:
		static void Frame(const char *name);

		static void Event(const char* functionName);
	
		static void EventDynamic(const char* name);

		static void Thread(const char* name);

		static void Shutdown();

	};
}

#define XYZ_ENABLE_PROFILING 1
#if XYZ_ENABLE_PROFILING
#define XYZ_PROFILE_FRAME(...)            ::XYZ::Profiler::Frame(__VA_ARGS__)
#define XYZ_PROFILE_FUNC(...)             ::XYZ::Profiler::Event(__VA_ARGS__)
#define XYZ_PROFILE_SCOPE_DYNAMIC(NAME)   ::XYZ::Profiler::EventDynamic(NAME)
#define XYZ_PROFILE_THREAD(NAME)          ::XYZ::Profiler::Thread(NAME)
#define XYZ_PROFILER_SHUTDOWN			  ::XYZ::Profiler::Shutdown();
#else
#define XYZ_PROFILE_FRAME(...)
#define XYZ_PROFILE_FUNC()
#define XYZ_PROFILE_TAG(NAME, ...) 
#define XYZ_PROFILE_SCOPE_DYNAMIC(NAME)
#define XYZ_PROFILE_THREAD(...)
#define XYZ_PROFILER_SHUTDOWN
#endif
