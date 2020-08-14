#pragma once
#include "System.h"
#include "Component.h"
#include "Types.h"

#include <memory>
#include <unordered_map>

namespace XYZ {
	
	/**
	*! @class SystemManager
	* @brief Manager for engine systems
	*/
	class SystemManager
	{
	public:
		/**
		* Register system of new Component T
		*/
		template<typename T>
		std::shared_ptr<T> RegisterSystem()
		{
			uint16_t id = System::GetID<T>();
			XYZ_ASSERT(m_Systems.find(id) == m_Systems.end(), "System already registered!");

			auto system = std::make_shared<T>();
			auto castedSystem = std::static_pointer_cast<System>(system);
			m_Systems.insert({ id,castedSystem });
			return system;
		}

		/**
		* Set signature of the system
		* @param[in] signature
		*/
		template<typename T>
		void SetSignature(Signature signature)
		{
			uint16_t id = System::GetID<T>();
			XYZ_ASSERT(m_Systems.find(id) != m_Systems.end(), "System is not registered!");

			m_Systems[id]->m_Signature = signature;
		}

		/**
		* Return the system of Component
		* @return shared_ptr o system
		*/
		template<typename T>
		std::shared_ptr<T> GetSystem()
		{
			uint16_t id = System::GetID<T>;
			XYZ_ASSERT(m_Systems.find(id) != m_Systems.end(), "System is not registered!");
			return std::static_pointer_cast<std::shared_ptr<T>>(m_Systems[id]);
		}

		/**
		* Removes entity from the corresponding systems
		* @param[in] entity
		* @param[in] entitySignature
		*/
		void EntityDestroyed(uint32_t entity, Signature entitySignature)
		{
			// Erase a destroyed entity from all system lists
			for (auto const& it : m_Systems)
			{
				auto const& Component = it.first;
				auto const& system = it.second;
				auto const& systemSignature = m_Systems[Component]->m_Signature;
				
				if ((entitySignature & systemSignature) == systemSignature)
				{	
					it.second->Remove(entity);
				}
			}
		}

		/**
		* Removes/Add entity from/to the system dependent on the signature
		* @param[in] entity
		* @param[in] entitySignature
		*/
		void EntitySignatureChanged(uint32_t entity, Signature entitySignature)
		{
			for (auto const& pair : m_Systems)
			{
				auto const& component = pair.first;
				auto const& system = pair.second;
				auto const& systemSignature = m_Systems[component]->m_Signature;

				if ((entitySignature & systemSignature) == systemSignature)
				{
					if (!system->Contains(entity))
						system->Add(entity);

				}
				else
					system->Remove(entity);

			}
		}
	private:
		std::unordered_map<uint16_t, std::shared_ptr<System>> m_Systems;
	};

}