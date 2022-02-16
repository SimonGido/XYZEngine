#pragma once

#include "ReflectionUtils.h"
#include "Reflectectables.h"

namespace XYZ {
	
	template<typename T>
	struct Reflection;

	#define REFLECTABLE(Type, ...) \
	\
	template<> \
	struct Reflection<Type> \
	{ \
		static constexpr size_t										   sc_NumVariables = NUMARGS(#__VA_ARGS__);\
		static constexpr const char*								   sc_VariablesString = #__VA_ARGS__;\
		static constexpr const char*								   sc_ClassName = #Type;\
		static constexpr std::array<std::string_view, sc_NumVariables> sc_VariableNames = Utils::SplitString<sc_NumVariables>(#__VA_ARGS__, ',', ' ');\
	public:\
		static auto ToTuple(Type&& s) \
		{\
			auto [__VA_ARGS__] = std::forward<Type>(s); \
			return std::make_tuple(__VA_ARGS__); \
		}\
		static auto ToReferenceTuple(Type& s) \
		{\
			auto &[__VA_ARGS__] = s; \
			return std::forward_as_tuple(__VA_ARGS__);\
		}\
		static const auto ToReferenceTuple(const Type& s) \
		{\
			auto &[__VA_ARGS__] = s; \
			return std::forward_as_tuple(__VA_ARGS__);\
		}\
		template <size_t Index>\
		static auto& Get(Type& s)\
		{\
			auto tmp = ToReferenceTuple(s);\
			return std::get<Index>(tmp);\
		}\
		template <size_t Index>\
		static const auto& Get(const Type& s)\
		{\
			auto tmp = ToReferenceTuple(s);\
			return std::get<Index>(tmp);\
		}\
		template <uint16_t Index>\
		static const auto& Get(const Type& s)\
		{\
			auto tmp = ToReferenceTuple(s);\
			return std::get<Index>(tmp);\
		}\
		template <typename T>\
		static T& GetByIndex(uint16_t index, Type& s)\
		{\
			auto tmp = ToReferenceTuple(s);\
			void* result = nullptr;\
			Utils::For([&](auto i) {\
				if (i.value == index)\
					result = &std::get<i.value>(tmp);\
			}, std::make_index_sequence<sc_NumVariables>());\
			return *static_cast<T*>(result);\
		}\
		template <typename T>\
		static const T& GetByIndex(uint16_t index, const Type& s)\
		{\
			auto tmp = ToReferenceTuple(s); \
			const void* result = nullptr;\
			Utils::For([&](auto i) {\
				if (i.value == index)\
					result = &std::get<i.value>(tmp); \
			}, std::make_index_sequence<sc_NumVariables>()); \
			return *static_cast<const T*>(result);\
		}\
		template <typename T>\
		static T& GetByName(const char* name, Type& s)\
		{\
			auto tmp = ToReferenceTuple(s);\
			T* result = nullptr;\
			size_t counter = 0;\
			std::apply([name, &result, &counter](auto&&... args) {\
				(assignFromName(name, (void**)&result, counter, &args), ...);\
			}, tmp);\
			return *result;\
		}\
	private:\
		static bool stringsEqual(const char* a, const char* b, size_t length)\
		{\
			for (size_t i = 0; i < length; ++i)\
				if (a[i] != b[i]) return false;\
			return true;\
		}\
		static void assignFromName(const char* name, void** result, size_t& counter, void* val)\
		{\
			if (sc_VariableNames[counter++] == name)\
				*result = val;\
		}\
	};
}