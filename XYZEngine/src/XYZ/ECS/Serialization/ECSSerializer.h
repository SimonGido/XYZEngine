#pragma once
#include "XYZ/ECS/ECSManager.h"
#include "ByteStream.h"

namespace XYZ {


	class ECSSerializer
	{
	public:
		template <typename ...ComponentTypes>
		static void Serialize(const ECSManager& ecs, ByteStream& out)
		{
			out << ecs.m_EntityManager.m_Signatures.Range();
			std::tuple<const ComponentStorage<ComponentTypes>&...> storages = { ecs.GetStorage<ComponentTypes>()... };
			ForEachInTuple(storages, [&](const auto& stor) {
				SerializeStorage(stor, out);
			});
		}
		template <typename ...ComponentTypes>
		static void Deserialize(ECSManager& ecs, const ByteStream& out)
		{
			size_t size = 0;
			out >> size;
			ecs.m_EntityManager.m_Signatures = FreeList<Signature>(size);
			std::tuple<const ComponentStorage<ComponentTypes>&...> storages = { ecs.GetStorage<ComponentTypes>()... };
			ForEachInTuple(storages, [&](auto& stor) {
				DeserializeStorage(stor, out);
			});
		}

		template <typename T>
		static void SerializeComponent(Entity entity, const T& component, ByteStream& out)
		{
			static_assert(std::is_base_of<IComponent, T>::value, "");
			out << (uint32_t)entity;
			out << component;
		}
		template <typename T>
		static void DeserializeComponent(Entity& entity, T& component, const ByteStream& out)
		{
			static_assert(std::is_base_of<IComponent, T>::value, "");
			out >> (uint32_t*)&entity;
			out >> component;
		}

		template <typename T>
		static void SerializeStorage(const ComponentStorage<T>& storage, ByteStream& out)
		{
			out << IComponent::GetComponentID<T>();
			out << storage.Size();
			uint32_t counter = 0;
			for (const T& component : storage)
				SerializeComponent<T>(storage.GetEntityAtIndex(counter++), component, out);
		}

		template <typename T>
		static void DeserializeStorage(ComponentStorage<T>& storage, const ByteStream& out)
		{
			uint8_t id;
			size_t size;
			out >> id;
			out >> size;
			for (size_t i = 0; i < size; ++i)
			{
				Entity entity;
				T component;
				DeserializeComponent<T>(entity, component, out);
				storage.AddComponent(entity, component);
			}
		}
	};

}