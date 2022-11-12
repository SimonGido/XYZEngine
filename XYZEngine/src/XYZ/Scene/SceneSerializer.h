#pragma once
#include "Scene.h"
#include "Components.h"

#include <future>

#include <yaml-cpp/yaml.h>

namespace XYZ {

	
	class XYZ_API SceneSerializer
	{
	public:
		void Serialize(const std::string& filepath, WeakRef<Scene> scene);
		Ref<Scene> Deserialize(const std::string& filepath);

	
	private:
		static void serializeEntity(YAML::Emitter& out, SceneEntity entity);
		static void deserializeEntity(YAML::Node& data, WeakRef<Scene> scene);

		template <typename T>
		static void serialize(YAML::Emitter& out, const T& val, SceneEntity entity);

		template <typename T>
		static void deserialize(YAML::Node& data, T& component, SceneEntity entity);

		template <typename T>
		static std::future<T> deserializeAsync(YAML::Node& data);


		template <typename T>
		static void serializeEntityComponent(YAML::Emitter& out, SceneEntity entity)
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
		static void serializeEntityComponents(YAML::Emitter& out, SceneEntity entity)
		{
			(serializeEntityComponent<Args>(out, entity), ...);
		}

	private:
		static void setupRelationship(YAML::Node& data, SceneEntity entity);
		static void setupAnimatedMeshComponent(YAML::Node& data, SceneEntity entity);

	};
}