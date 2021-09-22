#pragma once


namespace XYZ {
	namespace Reflect {
		template <size_t N>
		static std::array<std::string, N> SplitString(const std::string& string, const std::string& delimiters)
		{
			std::array<std::string, N> result;
			size_t start = 0;
			size_t end = string.find_first_of(delimiters);
			size_t counter = 0;
			while (end <= std::string::npos)
			{
				std::string token = string.substr(start, end - start);
				if (!token.empty())
					result[counter++] = token;

				if (end == std::string::npos)
					break;

				start = end + 1;
				end = string.find_first_of(delimiters, start);
			}
			return result;
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
	}
	#define NUMARGS(...) (Reflect::CountOccurances(#__VA_ARGS__, ',') + 1)

	#define REGISTER_REFLECTABLES(...)\
	\
	template <typename ...Args>\
	class Reflectables\
	{\
	public:\
		static constexpr size_t		sc_NumClasses	   = NUMARGS(#__VA_ARGS__);\
		static constexpr const char	sc_ClassesString[] = #__VA_ARGS__;\
		\
		\
		template <size_t Index>\
		static auto Get()\
		{\
			return std::get<Index>(sc_RegisteredClasses);\
		}\
		static std::array<std::string, sc_NumClasses>& GetClasses() \
		{\
			static auto classes = Reflect::SplitString<sc_NumClasses>(#__VA_ARGS__, ", ");\
			return classes;\
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
		static constexpr size_t		 sc_NumVariables = NUMARGS(#__VA_ARGS__);\
		static constexpr const char* sc_VariablesString = #__VA_ARGS__;\
		static constexpr const char* sc_ClassName = #Type;\
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
		template <size_t Index>\
		static auto& Get(Type& s)\
		{\
			auto tmp = ToReferenceTuple(s);\
			return std::get<Index>(tmp);\
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
		static std::array<std::string, sc_NumVariables>& GetVariables() \
		{\
			static auto variables = Reflect::SplitString<Reflection<Type>::sc_NumVariables>(#__VA_ARGS__, ", ");\
			return variables;\
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
			if (GetVariables()[counter++] == name)\
				*result = val;\
		}\
	};
}