#pragma once
#include "Types.h"
#include "Entity.h"

#include "XYZ/Utils/DataStructures/FreeList.h"

namespace XYZ {

	class EntityManager
	{
		friend class ECSManager;
	public:
		EntityManager();
		EntityManager(const EntityManager& other);
		EntityManager(EntityManager&& other) noexcept;

		Entity CreateEntity();

		Signature& GetSignature(Entity entity);
		const Signature& GetSignature(Entity entity)const;

		void DestroyEntity(Entity entity);
	
		void SetSignature(Entity entity, Signature signature);

		uint32_t GetNumEntities() const { return m_EntitiesInUse; }
	private:
		uint32_t m_EntitiesInUse;

		FreeList<Signature> m_Signatures;
		std::vector<bool> m_Valid;

		static constexpr uint32_t sc_MaxEntity = UINT32_MAX - 1;
		friend class ECSManager;
	};
}