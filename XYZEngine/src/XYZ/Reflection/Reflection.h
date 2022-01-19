#pragma once


namespace XYZ {
	namespace Reflect {
		template <std::size_t N, typename ...Args>
		constexpr std::array<std::string_view, N> SplitString(std::string_view str, Args ...delimiters)
		{
			auto compare = [](char a, char b) -> bool {
				return a == b;
			};
			std::array<std::string_view, N> arr{};
			std::size_t start = 0, end = 0;

			while ((compare(str[start], delimiters) || ...))
				start++;

			for (std::size_t i = 0; i < N && end != std::string_view::npos; i++)
			{
				end = std::string_view::npos;
				for (std::size_t j = start; j < str.length(); j++)
				{
					if ((compare(str[j], delimiters) || ...))
					{
						end = j;
						break;
					}
				}

				arr[i] = str.substr(start, end - start);
				std::size_t newStart = end + 1;
				while (newStart < str.length() && (compare(str[newStart], delimiters) || ...))
					newStart++;

				start = newStart;
			}

			return arr;
		}

		constexpr size_t CountOccurances(const char* str, char c)
		{
			return str[0] == char(0) ? 0 : (str[0] == c) + CountOccurances(str + 1, c);
		}

		template<std::size_t N>
		struct Num { static const constexpr auto value = N; };

		template <class F, std::size_t... Is>
		void For(F func, std::index_sequence<Is...>)
		{
			(func(Num<Is>{}), ...);
		}

		template <std::size_t I, class F>
		void For(F func)
		{
			For(func, std::make_index_sequence<I>());
		}

	}
	#define NUMARGS(...) (Reflect::CountOccurances(#__VA_ARGS__, ',') + 1)

	#define REGISTER_REFLECTABLES(...)\
	\
	template <typename ...Args>\
	class Reflectables\
	{\
	public:\
		static constexpr size_t										 sc_NumClasses	   = sizeof...(Args);\
		static constexpr const char									 sc_ClassesString[] = #__VA_ARGS__;\
		static constexpr std::array<std::string_view, sc_NumClasses> sc_ClassNames = Reflect::SplitString<sc_NumClasses>(#__VA_ARGS__, ',', ' ');\
		\
		\
		template <size_t Index>\
		static auto Get()\
		{\
			return std::get<Index>(sc_RegisteredClasses);\
		}\
	private:\
		static constexpr std::tuple<Reflection<Args>...> sc_RegisteredClasses = {}; \
	};\
	using ReflectedClasses = Reflectables<__VA_ARGS__>;


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
		static constexpr std::array<std::string_view, sc_NumVariables> sc_VariableNames = Reflect::SplitString<sc_NumVariables>(#__VA_ARGS__, ',', ' ');\
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
			Reflect::For([&](auto i) {\
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
			Reflect::For([&](auto i) {\
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