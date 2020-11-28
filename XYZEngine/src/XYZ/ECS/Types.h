#pragma once
#include <bitset>

namespace XYZ {
	#define MAX_ENTITIES 65536
	#define MAX_COMPONENTS 128
	#define NULL_ENTITY MAX_ENTITIES + 1
	#define HAS_GROUP_BIT 0

	using Signature = std::bitset<MAX_COMPONENTS>;


	template <int I, class... Ts>
	decltype(auto) get(Ts&&... ts)
	{
		return std::get<I>(std::forward_as_tuple(ts...));
	}
	template<typename... Types>
	constexpr auto GetTypesSize()
	{
		return std::array<std::size_t, sizeof...(Types)>{sizeof(Types)...};
	}

	template<class F, class...Ts, std::size_t...Is>
	void ForEachInTuple(const std::tuple<Ts...>& tuple, F func, std::index_sequence<Is...>)
	{
		using expander = int[];
		(void)expander {
			0, ((void)func(std::get<Is>(tuple)), 0)...
		};
	}

	template<class F, class...Ts>
	void ForEachInTuple(const std::tuple<Ts...>& tuple, F func)
	{
		ForEachInTuple(tuple, func, std::make_index_sequence<sizeof...(Ts)>());
	}
	
}