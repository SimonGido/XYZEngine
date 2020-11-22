#pragma once

#include <yaml-cpp/yaml.h>

namespace XYZ {
	class Serializer
	{
	public:
		template <typename T>
		static void Serialize(YAML::Emitter& out, const T& val);

	};
}