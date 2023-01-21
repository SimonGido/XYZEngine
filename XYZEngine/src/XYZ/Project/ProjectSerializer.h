#pragma once
#include "Project.h"

namespace XYZ {

	class XYZ_API ProjectSerializer
	{
	public:
		static void Serialize(const std::filesystem::path& filepath, Ref<Project> project);

		static Ref<Project> Deserialize(const std::filesystem::path& filepath);
	};
}
