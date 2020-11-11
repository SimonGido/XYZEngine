#pragma once

#include <bitset>

namespace XYZ {
	namespace ECS {

		class ComponentManagerT;
		class IComponentT
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

			friend class ComponentManagerT;
		};


		template <typename Derived, typename DeriveFrom = IComponentT>
		class TypeT : public IComponentT
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