#pragma once
#include "Pool.h"
#include "CallbackStorage.h"
#include "Component.h"
#include "Types.h"
#include "Signature.h"

namespace XYZ {

	class ECSManager;
	class CallbackManager
	{
	public:
		CallbackManager();
		CallbackManager(const CallbackManager& other);
		CallbackManager(CallbackManager&& other) noexcept;
		~CallbackManager();

		CallbackManager& operator=(CallbackManager&& other) noexcept;

		void Clear();

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
		
		ICallbackStorage* GetIStorage(size_t offset)
		{
			return m_StoragePool.Get<ICallbackStorage>(offset);
		}
		const ICallbackStorage* GetIStorage(size_t offset) const
		{
			return m_StoragePool.Get<ICallbackStorage>(offset);
		}

	private:
		template <typename T>
		CallbackStorage<T>* getOrCreateStorage()
		{
			size_t id = (size_t)IComponent::GetComponentID<T>();
			size_t offset = id * sizeof(CallbackStorage<T>);

			if (id >= m_Count)
				resizeStorages(id + 1 + sc_StorageCapacityInc);	
			if (!m_StorageCreated[id])
			{
				m_StoragePool.Allocate<CallbackStorage<T>>(offset);
				m_StorageCreated[id] = true;
				m_NumberOfStorages++;
			}
			return m_StoragePool.Get<CallbackStorage<T>>(offset);
		}

		void resizeStorages(size_t count);
		void deallocateStorages();
	private:
		Pool m_StoragePool;
		std::vector<bool> m_StorageCreated;
		size_t  m_Count;
		size_t  m_NumberOfStorages;
		static constexpr size_t sc_StorageCapacityInc = 5;
		static constexpr size_t sc_InitialStorageCapacity = 10;
	};
}