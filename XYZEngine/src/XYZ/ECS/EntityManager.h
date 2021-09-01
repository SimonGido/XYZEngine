#pragma once
#include "Types.h"
#include "Entity.h"
#include "Signature.h"
#include "DynamicBitset.h"

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

		Signature& GetSignature(Entity entity);
		const Signature& GetSignature(Entity entity)const;

		void DestroyEntity(Entity entity);
		void SetNumberOfComponents(uint16_t number);
		void SetSignature(Entity entity, Signature signature);
		void Clear();

		bool	 IsValid(Entity entity) const;
		uint32_t GetNumEntities() const { return m_EntitiesInUse; }

	private:
		uint32_t m_EntitiesInUse;

		DynamicBitset		  m_Bitset;
		std::vector<uint32_t> m_Versions;

		static constexpr uint32_t sc_MaxEntity = UINT32_MAX - 1;
		friend class ECSManager;
		friend class ECSSerializer;
	};
}