#pragma once

#include "ReflectionUtils.h"

namespace XYZ {
	#define REGISTER_REFLECTABLES(Name, ...)\
	\
	template <typename ...Args>\
	class Reflectables\
	{\
	public:\
		static constexpr size_t										 sc_NumClasses	   = sizeof...(Args);\
		static constexpr const char									 sc_ClassesString[] = #__VA_ARGS__;\
		static constexpr std::array<std::string_view, sc_NumClasses> sc_ClassNames = Utils::SplitString<sc_NumClasses>(#__VA_ARGS__, ',', ' ');\
		\
		\
		template <size_t Index>\
		static auto Get()\
		{\
			return std::get<Index>(sc_RegisteredClasses);\
		}\
	template <size_t Index>\
		static auto GetName()\
		{\
			std::array<char, sc_ClassNames[Index].size() + 1> result;					\
			memcpy(result.data(), sc_ClassNames[Index].data(), sc_ClassNames[Index].size());	\
			result[sc_ClassNames[Index].size()] = 0;						\
			return result;									\
		}\
	private:\
		static constexpr std::tuple<Reflection<Args>...> sc_RegisteredClasses = {}; \
	};\
	using Name = Reflectables<__VA_ARGS__>;
}