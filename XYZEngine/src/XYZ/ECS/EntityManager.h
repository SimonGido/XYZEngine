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

		EntityManager& operator=(EntityManager&& other) noexcept;

		Entity CreateEntity();

		uint32_t GetVersion(Entity entity) const;

		void DestroyEntity(Entity entity);
		void Clear();

		bool	 IsValid(Entity entity) const;
		uint32_t GetNumEntities() const { return m_EntitiesInUse; }

	private:
		uint32_t m_EntitiesInUse;

		FreeList<Entity>   m_Entities;
		FreeList<uint32_t> m_Versions;

		static constexpr uint32_t sc_MaxEntity = UINT32_MAX - 1;
		
		friend class ECSManager;
	};
}