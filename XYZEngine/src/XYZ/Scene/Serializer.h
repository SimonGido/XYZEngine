#pragma once
#include "XYZ/Core/Ref.h"

#include <yaml-cpp/yaml.h>


namespace XYZ {
	class AssetManager;
	class Serializer
	{
	public:
		template <typename T>
		static void Serialize(YAML::Emitter& out, const T& val);

		template <typename T>
		static T Deserialize(YAML::Node& data, AssetManager& assetManager);

		template <typename T>
		static void SerializeResource(const std::string& filepath, const Ref<T>& val);

		template <typename T>
		static Ref<T> DeserializeResource(const std::string& filepath, AssetManager& assetManager);
	};
}