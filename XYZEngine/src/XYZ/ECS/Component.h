#pragma once

#include <bitset>
#include <type_traits>
#include <limits>

namespace XYZ {

	template <typename T>
	class Component
	{
	public:
		static uint16_t ID();
		static bool     Registered();
	private:
		static uint16_t s_ID;

		friend class ComponentManager;
	};
	template <typename T>
	uint16_t Component<T>::s_ID = std::numeric_limits<uint16_t>::max();
	
	
	template<typename T>
	inline uint16_t Component<T>::ID()
	{
		XYZ_ASSERT(s_ID != std::numeric_limits<uint16_t>::max(), "Component type was not registered");
		return s_ID;
	}
	template<typename T>
	inline bool Component<T>::Registered()
	{
		return s_ID != std::numeric_limits<uint16_t>::max();
	}
}