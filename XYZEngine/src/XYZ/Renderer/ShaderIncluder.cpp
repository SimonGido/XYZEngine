#include "stdafx.h"
#include "ShaderIncluder.h"

#include "XYZ/Utils/FileSystem.h"
#include "XYZ/Utils/StringUtils.h"

#include <filesystem>

namespace XYZ {

	
	void ShaderIncluder::AddIncludes(const std::string& filepath)
	{
		processDirectory(std::filesystem::path(filepath));
	}

	void ShaderIncluder::AddIncludeFromFile(const std::string& filepath)
	{
		std::string path = filepath;
		std::replace(path.begin(), path.end(), '\\', '/');

		std::string content = FileSystem::ReadFile(path);

		XYZ_ASSERT(m_Includes.find(path) == m_Includes.end(), "Include already exists");

		m_Includes[path] = std::move(content);
	}
	void ShaderIncluder::processDirectory(const std::filesystem::path& path)
	{
		for (auto it : std::filesystem::directory_iterator(path))
		{
			if (it.is_directory())
			{
				processDirectory(it.path());
			}
			else if (Utils::GetExtension(it.path().string()) == "glsl")
			{
				AddIncludeFromFile(it.path().string());
			}
		}
	}
}