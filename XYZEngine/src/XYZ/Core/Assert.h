#pragma once
#include "Core.h"
#include "Logger.h"

#ifdef XYZ_ENABLE_ASSERTS

#define XYZ_ASSERT(x, ...) { if(!(x)) { XYZ_CORE_ERROR("Assertion Failed: {0}", __VA_ARGS__ ); DEBUG_BREAK; } }
#define XYZ_CHECK_THREAD(threadID) XYZ_ASSERT(std::this_thread::get_id() == threadID, "Wrong thread")

#else
#define XYZ_ASSERT(x, ...) 
#define XYZ_CHECK_THREAD(threadID)

#endif
