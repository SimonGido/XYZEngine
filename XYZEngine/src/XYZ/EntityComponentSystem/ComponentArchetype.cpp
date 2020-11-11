#include "stdafx.h"
#include "ComponentArchetype.h"

namespace XYZ {
	namespace ECS {
		ComponentArchetype::ComponentArchetype()
		{
			m_LayoutSize = 0;
		}
		ComponentArchetype::ComponentArchetype(const ComponentLayout& layout)
			:
			m_Layout(layout)
		{
			m_LayoutSize = 0;
			for (auto& it : layout.Elements)
			{
				m_Signature.set(it.ID);
				m_LayoutSize += it.Size;
			}
			m_Buffer.Allocate(m_LayoutSize);
		}
		ComponentArchetype::~ComponentArchetype()
		{
			
		}
		void ComponentArchetype::Clean()
		{
			delete[] m_Buffer;
		}
		void ComponentArchetype::AddComponent(uint32_t entity, Signature signature, uint8_t* component)
		{
			if (signature == m_Signature)
			{
				XYZ_ASSERT(m_EntityMap.find(entity) == m_EntityMap.end(), "Entity is already in this archetype");
				m_Entities.push_back(entity);
				if (m_Buffer.GetSize() < m_ElementsSize + m_LayoutSize)
					resizeBuffer();

				m_Buffer.Write(component, m_LayoutSize, m_ElementsSize);
				m_EntityMap[entity] = m_ElementsSize;
				m_ElementsSize += m_LayoutSize;
			}
		}
		bool ComponentArchetype::RemoveComponent(uint32_t entity, Signature signature)
		{
			if (signature == m_Signature)
			{
				auto it = m_EntityMap.find(entity);
				XYZ_ASSERT(it != m_EntityMap.end(), "Entity does not exist in this archetype");
				if (m_Entities.size() > 1)
				{
					// Update entities vector
					size_t index = it->second / m_LayoutSize;
					m_Entities[index] = m_Entities.back();
					m_Entities.pop_back();

					// Update byte buffer
					auto last = &m_Buffer[m_Buffer.GetSize() - m_LayoutSize];
					m_Buffer.Write(last, m_LayoutSize, it->second);

					// Update entitiy map
					m_EntityMap[m_Entities[index]] = it->second;
					m_EntityMap.erase(it);
					
					// Do not destroy archetype
					return false;
				}
			}
			// Destroy archetype
			return true;
		}
		uint8_t* ComponentArchetype::GetComponent(uint32_t entity)
		{
			auto it = m_EntityMap.find(entity);
			XYZ_ASSERT(it != m_EntityMap.end(), "Entity does not exist in this archetype");
			return &m_Buffer[it->second];
		}
		void ComponentArchetype::resizeBuffer()
		{
			auto size = m_Buffer.GetSize();

			ByteBuffer buffer;
			buffer.Allocate(size * 2);
			buffer.Write(m_Buffer, size);

			delete[]m_Buffer;
			m_Buffer = buffer;
		}
	}
}