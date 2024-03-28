#pragma once

#include "ReflectionUtils.h"
#include "Reflectables.h"

namespace XYZ {

	template<typename T>
	struct Reflection;

#define REFLECTABLE(Type, ...) \
	\
	template<> \
	struct Reflection<Type> \
	{ \
		using type = Type;\
		static constexpr size_t										   sc_NumVariables = NUMARGS(#__VA_ARGS__);\
		static constexpr const char*								   sc_VariablesString = #__VA_ARGS__;\
		static constexpr const char*								   sc_ClassName = #Type;\
		static constexpr std::array<std::string_view, sc_NumVariables> sc_VariableNames = Utils::SplitString<sc_NumVariables>(#__VA_ARGS__, ',', ' ');\
	public:\
		template <typename TFunc>\
		static void ForEach(const TFunc& func)\
		{\
			Utils::For<sc_NumVariables>([&](auto i) {\
				func(i);\
			});\
		}\
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
		\
		template <typename T>\
		static T& Get(Type& s, size_t index)\
		{\
			auto &[__VA_ARGS__] = s; \
			auto arr = Utils::ArgsToArray(__VA_ARGS__);\
			return *reinterpret_cast<T*>(arr[index]);\
		}\
		template <typename T>\
		static const T& Get(const Type& s, size_t index)\
		{\
			auto &[__VA_ARGS__] = s; \
			auto arr = Utils::ArgsToArray(__VA_ARGS__);\
			return *reinterpret_cast<const T*>(arr[index]);\
		}\
		\
		static auto GetVariablePointers(Type& s)\
		{\
			auto &[__VA_ARGS__] = s; \
			auto arr = Utils::ArgsToArray(__VA_ARGS__);\
			return arr;\
		}\
		\
		static const auto GetVariablePointers(const Type& s)\
		{\
			auto &[__VA_ARGS__] = s; \
			auto arr = Utils::ArgsToArray(__VA_ARGS__);\
			return arr;\
		}\
		\
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
		template <size_t Index>\
		static auto GetName()\
		{\
			std::array<char, sc_VariableNames[Index].size() + 1> result;					\
			memcpy(result.data(), sc_VariableNames[Index].data(), sc_VariableNames[Index].size());	\
			result[sc_VariableNames[Index].size()] = 0;						\
			return result;									\
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
		static auto deduceTupleRefType() \
		{\
			static constexpr Type* instance = nullptr;\
			auto &[__VA_ARGS__] = *instance; \
			return std::forward_as_tuple(__VA_ARGS__);\
		}\
		friend class ReflectionInstance<Type>;\
	};


	template <typename TRefl>
	class ReflectionInstance
	{
		using tuple_ref_type = decltype(Reflection<TRefl>::deduceTupleRefType());
		using variable_array_type = std::array<void*, Reflection<TRefl>::sc_NumVariables>;
	public:
		ReflectionInstance(TRefl& reflValue)
			:
			m_ReflectedValue(reflValue),
			m_Tuple(Reflection<TRefl>::ToReferenceTuple(reflValue))
		{
			m_Variables = Reflection<TRefl>::GetVariablePointers(reflValue);
		}

		template <size_t Index>
		auto& Get()
		{
			return std::get<Index>(m_Tuple);
		}

		template <size_t Index>
		const auto& Get() const
		{
			return std::get<Index>(m_Tuple);
		}

		template <typename T>
		T& Get(size_t index)
		{
			return *reinterpret_cast<T*>(m_Variables[index]);
		}

		template <typename T>
		const T& Get(size_t index) const
		{
			return *reinterpret_cast<const T*>(m_Variables[index]);
		}


	private:
		TRefl& m_ReflectedValue;
		variable_array_type m_Variables;
		tuple_ref_type		m_Tuple;
	};
}