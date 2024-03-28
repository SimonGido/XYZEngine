#pragma once
#include <memory>
#include <functional>
#include <map>
#include <thread>

#define RENDER_THREAD_ENABLED

#define _AMD64_

#ifdef _WIN32

#ifdef _WIN64
	#define XYZ_PLATFORM_WINDOWS
#elif __linux__
	#define XYZ_PLATFORM_LINUX
#else
	#error "x86 Builds are not supported!"
#endif

#endif

#ifdef XYZ_DEBUG
	#define XYZ_ENABLE_ASSERTS
	#ifdef _MSC_VER
		#define DEBUG_BREAK __debugbreak()
	#elif __GNUC__
		#include <signal.h>
		#define DEBUG_BREAK raise(0); 
	#endif
#endif


#define XYZ_EXPORT __declspec(dllexport)
#define XYZ_IMPORT __declspec(dllimport)

#ifdef XYZ_API_EXPORT
	#define XYZ_API XYZ_EXPORT
	
#elif XYZ_API_IMPORT
	#define XYZ_API XYZ_IMPORT
#else
	#define XYZ_API
#endif

#define BIND_EVENT_FN(x) std::bind(&Application::x, this, std::placeholders::_1)

#define BIT(x) (1 << x)
#define IS_SET(x, flag) ( x & (flag) )




namespace XYZ {
	template<typename T>
	using Scope = std::unique_ptr<T>;

	template<typename T, typename ... Args>
	constexpr Scope<T> CreateScope(Args&& ... args)
	{
		return std::make_unique<T>(std::forward<Args>(args)...);
	}

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

	template <typename T, typename... Rest>
	void HashCombine(size_t& seed, const T& v, Rest... rest)
	{
		std::hash<T> hasher;
		seed ^= hasher(v) + 0x9e3779b9 + (seed << 6) + (seed >> 2);
		(HashCombine(seed, rest), ...);
	}

	template <typename T>
	using vector2D = std::vector<std::vector<T>>;

	template <typename T>
	using vector3D = std::vector<vector2D<T>>;

	template <typename Key0, typename Key1, typename Value>
	using unordered_map2D = std::unordered_map<Key0, std::unordered_map<Key1, Value>>;

	template <typename Key0, typename Key1, typename Value>
	using map2D = std::map<Key0, std::map<Key1, Value>>;

	template <typename Key0, typename Key1, typename Key2, typename Value>
	using map3D = map2D<Key0, Key1, std::map<Key2, Value>>;

	template <std::size_t dimx, std::size_t dimy, typename T>
	using array2D = std::array<std::array<T, dimx>, dimy>;

	template <std::size_t dimx, std::size_t dimy, std::size_t dimz, typename T>
	using array3D = std::array<std::array<std::array<T, dimx>, dimy>, dimz>;

	template <std::size_t dim, typename T>
	using array_grid2D = array2D<dim, dim, T>;

	template <std::size_t dim, typename T>
	using array_grid3D = array3D<dim, dim, dim, T>;


	template <size_t size>
	struct  Padding
	{
		Padding() { memset(m_Bytes, 0, size); }
	private:
		char m_Bytes[size];
	};


	struct Bool32
	{
		Bool32() = default;
		Bool32(bool val)
			: m_Value(val)
		{}
		Bool32& operator=(bool other)
		{
			m_Value = other;
			return *this;
		}
		operator bool() const { return m_Value; }
	private:
		bool m_Value;
		Padding<3> m_Padding;
	};
}