#pragma once
#include "Types.h"
#include "Entity.h"

#include "XYZ/Utils/DataStructures/FreeList.h"

#include <queue>
namespace XYZ {

	class EntityManager
	{
		friend class ECSManager;
	public:
		EntityManager();
		EntityManager(const EntityManager& other);
		EntityManager(EntityManager&& other) noexcept;

		EntityManager& operator=(const EntityManager& other);
		EntityManager& operator=(EntityManager&& other) noexcept;

		Entity CreateEntity();


		void DestroyEntity(Entity entity);
		void Clear();

		bool	 IsValid(Entity entity) const;
		uint32_t GetNumEntities() const { return m_EntitiesInUse; }

	private:
		uint32_t m_EntitiesInUse;

		std::vector<Entity> m_Entities;
		std::queue<size_t>  m_Free;


		static constexpr uint32_t sc_MaxEntity = UINT32_MAX - 1;
		
		friend class ECSManager;
	};
}