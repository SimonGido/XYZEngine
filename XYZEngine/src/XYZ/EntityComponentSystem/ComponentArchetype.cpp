#include "stdafx.h"
#include "ComponentArchetype.h"

namespace XYZ {
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
		delete[]m_Buffer;
	}
	void ComponentArchetype::AddComponent(uint32_t entity, ECS::Signature signature, uint8_t* component)
	{
		if (signature == m_Signature)
		{
			XYZ_ASSERT(m_EntityMap.find(entity) == m_EntityMap.end(), "Entity is already in this archetype");
			if (m_Buffer.GetSize() < m_BufferSize + m_LayoutSize)
				resizeBuffer();

			m_Buffer.Write(component, m_LayoutSize, m_BufferSize);
			m_EntityMap[entity] = m_BufferSize;
			m_BufferSize += m_LayoutSize;
		}
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