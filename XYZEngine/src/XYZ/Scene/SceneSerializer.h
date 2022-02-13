#pragma once
#include "Scene.h"

#include <yaml-cpp/yaml.h>

namespace XYZ {

	class SceneSerializer
	{
	public:
		SceneSerializer(const Ref<Scene>& scene);

		void Serialize(const std::string& filepath);
		Ref<Scene> Deserialize(const std::string& filepath);

	private:
		void serializeEntity(YAML::Emitter& out, SceneEntity entity);

		template <typename T>
		void serialize(YAML::Emitter& out, const T& val, SceneEntity entity);

		template <typename T>
		void deserialize(YAML::Node& node, SceneEntity entity);

	private:
		Ref<Scene> m_Scene;
	};
}