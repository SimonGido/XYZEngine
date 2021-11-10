#pragma once
#include "Core.h"
#include "Logger.h"

#ifdef XYZ_ENABLE_ASSERTS

#define XYZ_ASSERT(x, ...) { if(!(x)) { XYZ_CORE_ERROR("Assertion Failed: {0}", __VA_ARGS__ ); DEBUG_BREAK; } }
#else
#define XYZ_ASSERT(x, ...) 


#endif
