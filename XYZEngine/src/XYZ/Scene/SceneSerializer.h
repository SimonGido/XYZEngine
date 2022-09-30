#pragma once
#include "Scene.h"

#include <yaml-cpp/yaml.h>

namespace XYZ {

	class SceneSerializer
	{
	public:
		void Serialize(const std::string& filepath, WeakRef<Scene> scene);
		Ref<Scene> Deserialize(const std::string& filepath);

	
	private:
		void serializeEntity(YAML::Emitter& out, SceneEntity entity);
		void deserializeEntity(YAML::Node& data, WeakRef<Scene> scene);

		template <typename T>
		void serialize(YAML::Emitter& out, const T& val, SceneEntity entity);

		template <typename T>
		void deserialize(YAML::Node& node, SceneEntity entity);

	private:
		static void setupRelationship(YAML::Node& data, SceneEntity entity);
		static void setupAnimatedMeshComponent(YAML::Node& data, SceneEntity entity);

	};
}