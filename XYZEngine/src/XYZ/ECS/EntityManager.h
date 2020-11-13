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
			const Signature& GetSignature(uint32_t entity)const;

			void DestroyEntity(uint32_t entity);
	
			void SetSignature(uint32_t entity, Signature signature);

			uint32_t GetNumEntities() const { return m_EntitiesInUse; }
		private:
			uint32_t m_EntitiesInUse;

			FreeList<Signature> m_Signatures;
		};
	}
}