#pragma once
#include "Types.h"

#include "XYZ/Utils/DataStructures/ByteBuffer.h"

namespace XYZ {
	struct ComponentElement
	{
		size_t  Size;
		uint8_t ID;
	};

	struct ComponentLayout
	{
		std::vector<ComponentElement> Elements;
	};

	class ComponentArchetype
	{
	public:
		ComponentArchetype(const ComponentLayout& layout);
		~ComponentArchetype();

		void AddComponent(uint32_t entity, ECS::Signature signature, uint8_t* component);
		
		template <typename ...Args>
		std::tuple<Args...>& GetComponent(uint32_t entity)
		{
			auto it = m_EntityMap.find(entity);
			XYZ_ASSERT(it != m_EntityMap.end(), "Entity does not exist in this archetype");

			return *reinterpret_cast<std::tuple<Args...>*>(&m_Buffer[it->second]);
		}

	private:
		void resizeBuffer();

	private:
		ComponentLayout m_Layout;


		ECS::Signature m_Signature;
		ByteBuffer m_Buffer;
		size_t m_LayoutSize;

		size_t m_BufferSize = 0;
		std::unordered_map<uint32_t, size_t> m_EntityMap; // stores entity and memory offset in buffer
	};
}