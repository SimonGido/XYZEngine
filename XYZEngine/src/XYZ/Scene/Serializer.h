#pragma once

#include <yaml-cpp/yaml.h>

namespace XYZ {
	class Serializer
	{
	public:
		template <typename T>
		static void Serialize(YAML::Emitter& out, const T& val);

		template <typename T>
		static T Deserialize(YAML::Node& data);
	};
}