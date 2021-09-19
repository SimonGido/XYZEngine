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


		#define REFLECTABLE(Type, ...) \
		\
		template<typename Type> \
		struct Reflection \
		{ \
		\
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
			static constexpr size_t GetNumberOfVariables() { return sc_NumVariables; }\
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
			static constexpr const char* GetClassName() { return sc_ClassName; }\
		private:\
			static constexpr std::size_t delimiterCount(char const* str, char sep)\
			{\
				std::size_t count{ 1 };\
				while (*str)\
					if (sep == *str++) ++count;\
				return count;\
			}\
			static bool stringsEqual(const char* a, const char* b, size_t length)\
			{\
				for (size_t i = 0; i < length; ++i)\
					if (a[i] != b[i]) return false;\
				return true;\
			}\
			static void assignFromName(const char* name, void** result, size_t& counter, void* val)\
			{\
				if (s_VariableNames[counter++] == name)\
					*result = val;\
			}\
		private:\
			static constexpr size_t							sc_NumVariables = delimiterCount(#__VA_ARGS__, ',');\
			static constexpr const char*					sc_VariablesString = #__VA_ARGS__;\
			static constexpr const char*					sc_ClassName = #Type;\
			static std::array<std::string, sc_NumVariables>	s_VariableNames;\
		};\
		std::array<std::string, Reflection<Type>::GetNumberOfVariables()> Reflection<Type>::s_VariableNames = Reflect::SplitString<Reflection<Type>::GetNumberOfVariables()>(#__VA_ARGS__, ", ");
		

	}
}