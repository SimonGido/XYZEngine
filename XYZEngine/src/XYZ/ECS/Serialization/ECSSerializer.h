#pragma once
#include "XYZ/ECS/ECSManager.h"
#include "ByteStream.h"

#include <vector>

namespace XYZ {

	struct ECSHeader
	{
		uint32_t NumEntities;
		bool	 EntitiesIncluded;
		bool	 Tight;

		static constexpr uint8_t TightPackLength = 8;
	};

	inline ByteStream& operator <<(ByteStream& out, const ECSHeader& header)
	{
		out << header.NumEntities << header.EntitiesIncluded << header.Tight;
		return out;
	}

	inline const ByteStream& operator >>(const ByteStream& in, ECSHeader& header)
	{
		in >> header.NumEntities;
		in >> header.EntitiesIncluded;
		in >> header.Tight;
		return in;
	}

	class ECSSerializer
	{
	public:
		// Serialize everything, in order of component storages
		static void Serialize(const ECSManager& ecs, ByteStream& out, bool tight = true);

		// Resolves entities and component types and deserialize them, does not require data to be in order
		static void Deserialize(ECSManager& ecs, const ByteStream& in);


		template <typename ...ComponentTypes>
		static void Serialize(const ECSManager& ecs, ByteStream& out, bool tight = true)
		{
			serializeECSHeaderAndData(ecs, out, tight);
			(SerializeStorage<ComponentTypes>(ecs, out),...);
		}

		template <typename ...ComponentTypes>
		static void Deserialize(ECSManager& ecs, const ByteStream& in)
		{
			deserializeECSHeaderAndData(ecs, in);
			(DeserializeStorage<ComponentTypes>(ecs, in),...);
		}

		// If writeInfo is true it will write component id and number of components ( 1 ), if you want to write custom info set it to false
		static void SerializeComponent(const ECSManager& ecs, Entity entity, uint16_t componentID, ByteStream& out, bool writeInfo = true);

		// If writeInfo is true it will write component id and number of components ( 1 ), if you want to write custom info set it to false
		template <typename T>
		static void SerializeComponent(Entity entity, const T& component, ByteStream& out, bool writeInfo = true)
		{
			static_assert(std::is_base_of<IComponent, T>::value, "");
			if (writeInfo)
			{
				out << Component<T>::ID();
				out << (size_t)1;
			}
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
		static void SerializeStorage(const ECSManager& ecs, ByteStream& out)
		{
			const ComponentStorage<T>& storage = ecs.GetStorage<T>();
			out << Component<T>::ID();
			out << storage.Size();
			uint32_t counter = 0;
			for (const T& component : storage)
				SerializeComponent<T>(storage.GetEntityAtIndex(counter++), component, out);
		}

		template <typename T>
		static void DeserializeStorage(ECSManager& ecs, const ByteStream& in)
		{
			uint16_t id;
			size_t size;
			in >> id;
			in >> size;
			for (int32_t i = 0; i < size; ++i)
			{
				Entity entity;
				T component;
				DeserializeComponent<T>(entity, component, in);
				entity = ecs.CreateEntity();
				ecs.AddComponent(entity, component);
			}
		}	
	private:
		static void serializeECSHeaderAndData(const ECSManager& ecs, ByteStream& out, bool tight);
		static void deserializeECSHeaderAndData(ECSManager& ecs, const ByteStream& in);
	};

}