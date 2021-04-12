#pragma once
#include "XYZ/ECS/ECSManager.h"
#include "ByteStream.h"

namespace XYZ {


	class ECSSerializer
	{
	public:
		// Serialize everything, in order of component storages
		static void SerializeRaw(const ECSManager& ecs, ByteStream& out);

		// Resolves entities and component types and deserialize them, does not require data to be in order
		static void DeserializeRaw(ECSManager& ecs, const ByteStream& in);

		template <typename ...ComponentTypes>
		static void Serialize(const ECSManager& ecs, ByteStream& out)
		{
			out << ecs.m_EntityManager.m_Bitset.GetNumberOfSignatures();
			std::tuple<const ComponentStorage<ComponentTypes>&...> storages = { ecs.GetStorage<ComponentTypes>()... };
			ForEachInTuple(storages, [&](const auto& stor) {
				SerializeStorage(stor, out);
			});
		}
		template <typename ...ComponentTypes>
		static void Deserialize(ECSManager& ecs, const ByteStream& in)
		{
			size_t size = 0;
			in >> size;
			ecs.m_EntityManager.m_Signatures = FreeList<Signature>(size);
			std::tuple<const ComponentStorage<ComponentTypes>&...> storages = { ecs.GetStorage<ComponentTypes>()... };
			ForEachInTuple(storages, [&](auto& stor) {
				DeserializeStorage(stor, in);
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
		static void DeserializeComponent(Entity& entity, T& component, const ByteStream& in)
		{
			static_assert(std::is_base_of<IComponent, T>::value, "");
			in >> *(uint32_t*)&entity;
			in >> component;
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
		static void DeserializeStorage(ComponentStorage<T>& storage, const ByteStream& in)
		{
			uint16_t id;
			size_t size;
			in >> id;
			in >> size;
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