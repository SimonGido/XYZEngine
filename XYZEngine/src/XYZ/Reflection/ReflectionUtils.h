#pragma once

#include <array>
#include <stdint.h>
#include <string>

namespace XYZ {
	namespace Utils {
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

		
		#define NUMARGS(...) (Utils::CountOccurances(#__VA_ARGS__, ',') + 1)
	}
}