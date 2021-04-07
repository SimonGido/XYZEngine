#pragma once
#include "Pool.h"
#include "CallbackStorage.h"
#include "Component.h"
#include "Types.h"

namespace XYZ {

	class ECSManager;
	class CallbackManager
	{
	public:
		CallbackManager();
		CallbackManager(const CallbackManager& other);
		CallbackManager(CallbackManager&& other) noexcept;
		~CallbackManager();

		template <typename T>
		void AddListener(const std::function<void(uint32_t, CallbackType)>& callback, void* instance)
		{
			CallbackStorage<T>* storage = getOrCreateStorage<T>();
			storage->AddListener(callback, instance);
		}

		template <typename T>
		void RemoveListener(void* instance)
		{
			CallbackStorage<T>* storage = getOrCreateStorage<T>();
			storage->RemoveListener(instance);
		}

		template <typename T>
		void OnComponentCreate(uint32_t entity)
		{
			CallbackStorage<T>* storage = getOrCreateStorage<T>();
			storage->Execute(entity, CallbackType::ComponentCreate);
		}

		template <typename T>
		void OnComponentRemove(uint32_t entity)
		{
			CallbackStorage<T>* storage = getOrCreateStorage<T>();
			storage->Execute(entity, CallbackType::ComponentRemove);
		}

		void OnEntityDestroyed(uint32_t entity, const Signature& signature);
		
		const ICallbackStorage* GetIStorage(size_t offset) const
		{
			return m_StoragePool.Get<ICallbackStorage>(offset);
		}

	private:
		template <typename T>
		CallbackStorage<T>* getOrCreateStorage()
		{
			uint8_t id = IComponent::GetComponentID<T>();
			if (!m_StorageCreated[id])
			{
				m_StoragePool.Allocate<CallbackStorage<T>>((size_t)id * sizeof(CallbackStorage<T>));
				m_StorageCreated[id] = true;
				m_NumberOfStorages++;
			}
			return m_StoragePool.Get<CallbackStorage<T>>((size_t)id * sizeof(CallbackStorage<T>));
		}
		
	private:
		Pool m_StoragePool;
		std::vector<bool> m_StorageCreated;
		size_t m_NumberOfStorages = 0;
	};
}