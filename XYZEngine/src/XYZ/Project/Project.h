#pragma once

#include <filesystem>

namespace XYZ {

	enum class PluginLanguage
	{
		None,
		CPP,
		CS
	};
	
	struct Plugin
	{
		std::string			  PluginName;
		std::filesystem::path PluginDirectory;
		PluginLanguage		  Language;

		std::string BinaryPath() const;
	};

	struct ProjectConfig
	{
		enum Flags
		{
			VR = BIT(0)
		};

		std::string Name = "Untitled";

		std::filesystem::path StartScene;

		std::filesystem::path AssetDirectory;

		std::vector<Plugin> Plugins;

		uint32_t Flags = 0;
	};

	class XYZ_API Project : public RefCount
	{
	public:		
		static Ref<Project> GetActive();

		static Ref<Project> New();
		static Ref<Project> Load(const std::filesystem::path& path);

		static bool SaveActive(const std::filesystem::path& path);


		void ReloadPlugins();

		ProjectConfig Configuration;

	private:
		std::filesystem::path m_ProjectDirectory;
	};
}