#pragma once

#include <bitset>

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
		static uint8_t getNextID()
		{
			static uint8_t nextType = 0;
			return nextType++;
		}
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

		virtual void Copy(IComponent* component) = 0;

		template <typename T>
		static uint8_t GetComponentID()
		{
			return Type<T>::GetComponentID();
		}
	};
}