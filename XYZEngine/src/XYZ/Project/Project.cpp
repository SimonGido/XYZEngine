#include "stdafx.h"
#include "Project.h"

#include "ProjectSerializer.h"

namespace XYZ {
	static Ref<Project> s_ActiveProject;

	Ref<Project> Project::GetActive()
	{
		return s_ActiveProject;
	}
	Ref<Project> Project::New()
	{
		s_ActiveProject = Ref<Project>::Create();
		return s_ActiveProject;
	}
	Ref<Project> Project::Load(const std::filesystem::path& path)
	{
		// Deserialize

		return s_ActiveProject;
	}
	bool Project::SaveActive(const std::filesystem::path& path)
	{
		ProjectSerializer::Serialize(path, s_ActiveProject);
		s_ActiveProject->m_ProjectDirectory = path.parent_path();
		return true;
	}
}