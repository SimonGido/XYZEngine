#pragma once

#include <bitset>
#include <type_traits>

namespace XYZ {

	class Counter
	{
	public:
		template <typename T>
		static uint8_t GetID()
		{
			static uint8_t compType = getNextID();
			return compType;
		}
	
	private:
		static uint8_t getNextID();
		
	};


	template <typename Derived, typename DeriveFrom = Counter>
	class Type : public Counter
	{
	public:
		// return unique static id
		static uint8_t GetComponentID()
		{
			return Counter::GetID<Derived>();
		}
	};


	class IComponent
	{
	public:
		virtual ~IComponent() {}

		template <typename T>
		static uint8_t GetComponentID()
		{
			static_assert(std::is_base_of<IComponent, T>::value, "Type T does not inherit from IComponent");
			return Type<T>::GetComponentID();
		}
	};
}