#pragma once
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
		void CreateStorage()
		{
			createStorage<T>();
		}

		template <typename T>
		void AddListener(const std::function<void(uint32_t, CallbackType)>& callback, void* instance)
		{
			createStorage<T>();
			CallbackStorage<T>& storage = GetStorage<T>();
			storage.AddListener(callback, instance);
		}

		template <typename T>
		void RemoveListener(void* instance)
		{
			createStorage<T>();
			CallbackStorage<T>& storage = GetStorage<T>();
			storage.RemoveListener(instance);
		}

		template <typename T>
		void OnComponentCreate(uint32_t entity)
		{
			createStorage<T>();
			CallbackStorage<T>& storage = GetStorage<T>();
			storage.Execute(entity, CallbackType::ComponentCreate);
		}

		template <typename T>
		void OnComponentRemove(uint32_t entity)
		{
			createStorage<T>();
			CallbackStorage<T>& storage = GetStorage<T>();
			storage.Execute(entity, CallbackType::ComponentRemove);
		}

		void OnEntityDestroyed(uint32_t entity, const Signature& signature);
		
		template <typename T>
		CallbackStorage<T>& GetStorage()
		{
			XYZ_ASSERT(m_Storages[(size_t)Component<T>::ID()], "Storage is not initialized");
			return *static_cast<CallbackStorage<T>*>(m_Storages[(size_t)Component<T>::ID()]);
		}

		template <typename T>
		const CallbackStorage<T>& GetStorage() const
		{
			XYZ_ASSERT(m_Storages[(size_t)Component<T>::ID()], "Storage is not initialized");
			return *static_cast<CallbackStorage<T>*>(m_Storages[(size_t)Component<T>::ID()]);
		}

		ICallbackStorage* GetIStorage(uint16_t index)
		{
			return m_Storages[(size_t)index];
		}
		const ICallbackStorage* GetIStorage(uint16_t index) const
		{
			return m_Storages[(size_t)index];
		}
	private:
		template <typename T>
		void createStorage()
		{
			size_t oldSize = m_Storages.size();
			size_t id = (size_t)Component<T>::ID();
			if (oldSize <= id)
				m_Storages.resize(id + 1);
			for (size_t i = oldSize; i < m_Storages.size(); ++i)
				m_Storages[i] = nullptr;
			if (m_Storages[id]) // Storage already exists
				return;

			m_Storages[id] = new CallbackStorage<T>();
			m_StoragesCreated++;
		}


		void destroyStorages();

	private:
		std::vector<ICallbackStorage*>  m_Storages;
		uint16_t						m_StoragesCreated;
	};
}