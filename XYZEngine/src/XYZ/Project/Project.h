#pragma once

#include <filesystem>

namespace XYZ {

	struct ProjectConfig
	{
		std::string Name = "Untitled";

		std::filesystem::path StartScene;

		std::filesystem::path AssetDirectory;
		std::filesystem::path ScriptModulePath;
	};

	class XYZ_API Project : public RefCount
	{
	public:		
		static Ref<Project> GetActive();

		static Ref<Project> New();
		static Ref<Project> Load(const std::filesystem::path& path);

		static bool SaveActive(const std::filesystem::path& path);

		ProjectConfig Configuration;

	private:
		std::filesystem::path m_ProjectDirectory;
	};
}