#pragma once

#include <bitset>

namespace XYZ {
	namespace ECS {

		class ComponentManager;
		class IComponent
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
				static uint16_t nextType = 0;
				return ++nextType;
			}

			friend class ComponentManager;
		};


		template <typename Derived, typename DeriveFrom = IComponent>
		class Type : public IComponent
		{
		public:
			// return unique static id
			static uint8_t GetComponentID()
			{
				return IComponent::GetID<Derived>();
			}
		};
	}
}