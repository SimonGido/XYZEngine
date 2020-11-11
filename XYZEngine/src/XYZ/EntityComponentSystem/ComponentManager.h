#pragma once
#include "ComponentArchetype.h"
#include "Component.h"

namespace XYZ {
	namespace ECS {

		class ComponentManagerT
		{
		public:
			template <typename T>
			void AddComponent(uint32_t entity, Signature& signature, const T& component)
			{		
				ByteBuffer tempStorage;
				ComponentLayout layout;		
				layout.Elements.push_back({ sizeof(T), T::GetComponentID() });
				// Remove entity from old archetype
				auto it = m_ArcheTypes.find(m_HashFunction(signature));
				if (it != m_ArcheTypes.end())
				{
					// Storing old entity component data and merging them with new component
					tempStorage.Allocate(it->second.GetLayoutSize() + sizeof(T));
					tempStorage.Write((void*)&component, sizeof(T));
					tempStorage.Write(it->second.GetComponent(entity), it->second.GetLayoutSize(), sizeof(T));
					for (const ComponentElement& elem : it->second.GetLayout().Elements)
						layout.Elements.push_back(elem);

					if (it->second.RemoveComponent(entity, signature))
					{
						// It returned true, means archetype is empty -> destroy it
						it->second.Clean();
						m_ArcheTypes.erase(it);
					}
				}
				else
				{
					// Old Archetype did not exist
					tempStorage.Allocate(sizeof(T));
					tempStorage.Write((void*)&component, sizeof(T));
				}

				
				// Update entity signature to hold new component
				signature.set(T::GetComponentID());

				// Find if this archetype already exists
				auto updatedHash = m_HashFunction(signature);
				auto newIt = m_ArcheTypes.find(updatedHash);
				if (newIt != m_ArcheTypes.end())
				{
					// New archetype exists so add component
					newIt->second.AddComponent(entity, signature, tempStorage);
				}
				else
				{
					// New arhcetype does not exists so create new archetype and add component
					m_ArcheTypes[updatedHash] = ComponentArchetype(layout);
					m_ArcheTypes[updatedHash].AddComponent(entity, signature, tempStorage);
				}

				delete[]tempStorage;
			}

			ComponentArchetype& GetArchetype(const Signature& signature) { return m_ArcheTypes[m_HashFunction(signature)]; }

		private:
			std::hash<std::bitset<MAX_COMPONENTS>> m_HashFunction;

			std::unordered_map<size_t, ComponentArchetype> m_ArcheTypes;
	

			uint8_t m_NextID = 1;
		};
	}
}