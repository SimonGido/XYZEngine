#pragma once
#include <memory>
#include <functional>

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




#ifdef XYZ_ENABLE_ASSERTS
#define XYZ_ASSERT(x, ...) { if(!(x)) { XYZ_LOG_ERR("Assertion Failed: ", __VA_ARGS__ ); __debugbreak(); } }
#else
#define XYZ_ASSERT(x, ...) 
#endif


#ifndef __FUNCTION__
#define __FUNCTION__ "Function:"
#endif


namespace XYZ {
	template <typename T>
	using Ref = std::shared_ptr<T>;

	template <typename T, typename ...Args>
	constexpr Ref<T> CreateRef(Args&&...args)
	{
		return std::make_shared<T>(std::forward<Args>(args)...);
	};


	template<typename R, typename T, typename U, typename... Args>
	constexpr std::function<R(Args...)> Hook(R(T::* f)(Args...), U p)
	{
		return [p, f](Args... args)->R { return (p->*f)(args...); };
	};

}