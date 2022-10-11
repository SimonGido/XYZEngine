#pragma once
#include "Scene.h"
#include "Components.h"

#include <future>

#include <yaml-cpp/yaml.h>

namespace XYZ {

	template <typename ...Args>
	struct MultiComponent
	{
		template <typename T>
		struct Optional
		{
			T	 Value;
			bool Used = false;
		};

		SceneEntity Entity;

		void AddToEntity()
		{
			(addComponentToEntity<Args>(), ...);
		}

		template <typename T>
		T& Get()
		{
			Optional<T>& val = std::get<Optional<T>>(m_Values);
			val.Used = true;
			return val.Value;
		}

		template <typename T>
		bool Used() const
		{
			return std::get<Optional<T>>(m_Values).Used;
		}
	private:
		template <typename T>
		void addComponentToEntity()
		{
			Optional<T>& val = std::get<Optional<T>>(m_Values);
			if (val.Used)
			{
				if (Entity.HasComponent<T>())
				{
					Entity.GetComponent<T>() = val.Value;
				}
				else
				{
					Entity.AddComponent<T>(val.Value);
				}
			}
		}

	private:
	

		std::tuple<Optional<Args>...> m_Values;
	};


	class SceneSerializer
	{
	public:
		void Serialize(const std::string& filepath, WeakRef<Scene> scene);
		Ref<Scene> Deserialize(const std::string& filepath);

	
	private:
		void serializeEntity(YAML::Emitter& out, SceneEntity entity);
		void deserializeEntity(YAML::Node& data, WeakRef<Scene> scene);

		std::future<MultiComponent<XYZ_COMPONENTS>> deserializeEntityAsync(YAML::Node& data, WeakRef<Scene> scene);

		template <typename T>
		void serialize(YAML::Emitter& out, const T& val, SceneEntity entity);

		template <typename T>
		void deserialize(YAML::Node& data, T& component, SceneEntity entity);

		template <typename T>
		std::future<T> deserializeAsync(YAML::Node& data);


		template <typename T>
		void serializeEntityComponent(YAML::Emitter& out, SceneEntity entity)
		{
			if constexpr (!std::is_same_v<T, IDComponent>)
			{
				if (entity.HasComponent<T>())
				{
					serialize<T>(out, entity.GetComponent<T>(), entity);
				}
			}
		}
		template <typename ...Args>
		void serializeEntityComponents(YAML::Emitter& out, SceneEntity entity)
		{
			(serializeEntityComponent<Args>(out, entity), ...);
		}

	private:
		static void setupRelationship(YAML::Node& data, SceneEntity entity);
		static void setupAnimatedMeshComponent(YAML::Node& data, SceneEntity entity);

	};
}