#pragma once
#include "XYZ/Core/Core.h"

#include <optick.h>


#define XYZ_ENABLE_PROFILING 1
#if XYZ_ENABLE_PROFILING
#define XYZ_PROFILE_FRAME(...)            OPTICK_FRAME(__VA_ARGS__);
#define XYZ_PROFILE_FUNC(...)             OPTICK_EVENT(__VA_ARGS__)
#define XYZ_PROFILE_SCOPE_DYNAMIC(NAME)   OPTICK_EVENT_DYNAMIC(NAME)
#define XYZ_PROFILE_THREAD(NAME)          OPTICK_THREAD(NAME)
#define XYZ_PROFILER_SHUTDOWN()			  OPTICK_SHUTDOWN();
#else
#define XYZ_PROFILE_FRAME(...)
#define XYZ_PROFILE_FUNC()
#define XYZ_PROFILE_TAG(NAME, ...) 
#define XYZ_PROFILE_SCOPE_DYNAMIC(NAME)
#define XYZ_PROFILE_THREAD(...)
#define XYZ_PROFILER_SHUTDOWN
#endif
