#pragma once
#include "Types.h"
#include "XYZ/Utils/DataStructures/FreeList.h"

#include <queue>
#include <array>

namespace XYZ {
	namespace ECS {
		
		class EntityManager
		{
			friend class ECSManager;
		public:
	
			EntityManager();

	
			uint32_t CreateEntity();

			Signature& GetSignature(uint32_t entity);
	
			void DestroyEntity(uint32_t entity);
	
			void SetSignature(uint32_t entity, Signature signature);

			uint32_t GetNumEntitiesInUse() const { return m_EntitiesInUse; }
		private:
			uint32_t m_EntitiesInUse;

			struct EntitySignature
			{
				Signature Signature;
				uint32_t Entity;
			};

			// Every entity has Signature representing components that it contains
			// System with same signatures will use entity
			FreeList<EntitySignature> m_Signatures;
		};
	}
}