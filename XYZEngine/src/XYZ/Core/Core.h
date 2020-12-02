#pragma once
#include <memory>
#include <functional>

#define _AMD64_

#ifdef _WIN32

#ifdef _WIN64
	#define XYZ_PLATFORM_WINDOWS
#else
	#error "x86 Builds are not supported!"
#endif

#endif

#ifdef XYZ_DEBUG
#define XYZ_ENABLE_ASSERTS
#endif

#define BIND_EVENT_FN(x) std::bind(&Application::x, this, std::placeholders::_1)

#define BIT(x) (1 << x)


#ifdef XYZ_ENABLE_ASSERTS
#define XYZ_ASSERT(x, ...) { if(!(x)) { XYZ_LOG_ERR("Assertion Failed: ", __VA_ARGS__ ); __debugbreak(); } }
#else
#define XYZ_ASSERT(x, ...) 
#endif


#ifndef __FUNCTION__
#define __FUNCTION__ "Function:"
#endif


namespace XYZ {

	template<typename R, typename T, typename U, typename... Args>
	constexpr std::function<R(Args...)> Hook(R(T::* f)(Args...), U p)
	{
		return [p, f](Args... args)->R { return (p->*f)(args...); };
	};


	template <typename E>
	constexpr auto ToUnderlying(E e) noexcept
	{
		return static_cast<std::underlying_type_t<E>>(e);
	}
}