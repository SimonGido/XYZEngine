#pragma once
#include "Types.h"

#include "XYZ/Utils/DataStructures/ByteBuffer.h"

namespace XYZ {
	namespace ECS {

	
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
			ComponentArchetype();
			ComponentArchetype(const ComponentLayout& layout);
			~ComponentArchetype();

			void Clean();

			void AddComponent(uint32_t entity, Signature signature, uint8_t* component);
			
			bool RemoveComponent(uint32_t entity, Signature signature);
	
			uint8_t* GetComponent(uint32_t entity);

			template <typename ...Args>
			std::tuple<Args...>* GetComponents(size_t& count)
			{
				count = m_Entities.size();
				return m_Buffer.As<std::tuple<Args...>>();
			}
			
			template <typename ...Args>
			std::tuple<Args...>& GetComponentsAs(uint32_t entity)
			{
				auto it = m_EntityMap.find(entity);
				XYZ_ASSERT(it != m_EntityMap.end(), "Entity does not exist in this archetype");


				return *reinterpret_cast<std::tuple<Args...>*>(&m_Buffer[it->second]);
			}

			
			template <typename T>
			T& GetComponentAs(uint32_t entity)
			{
				auto it = m_EntityMap.find(entity);
				XYZ_ASSERT(it != m_EntityMap.end(), "Entity does not exist in this archetype");

				size_t offset = 0;
				for (auto& el : m_Layout.Elements)
				{
					if (el.ID == T::GetComponentID())
						return *(T*)&m_Buffer[it->second + offset];
					
					offset += el.Size;
				}
			}

			const ComponentLayout& GetLayout() const { return m_Layout; }
			const Signature& GetSignature() const { return m_Signature; }
			const size_t GetLayoutSize() const { return m_LayoutSize; }
		private:
			void resizeBuffer();

		private:
			ComponentLayout m_Layout;

			Signature m_Signature;
			ByteBuffer m_Buffer;
			size_t m_LayoutSize;

			size_t m_ElementsSize = 0;
			std::vector<uint32_t> m_Entities;
			std::unordered_map<uint32_t, size_t> m_EntityMap; // stores entity and memory offset in buffer
		};
	}
}