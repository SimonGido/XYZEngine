#pragma once
#include <filesystem>

namespace XYZ {
	namespace Utils {

		enum class ProjectKind
		{
			SharedLib,
			StaticLib,
			Executable
		};

		enum class CppDialect
		{
			CPP14,
			CPP17,
			CPP20
		};

		struct PremakeProject
		{
			std::string				 Name;
			std::filesystem::path	 TargetPath;
			std::filesystem::path	 ObjPath;
			std::vector<std::string> IncludedModules;
			CppDialect				 Dialect;
			ProjectKind				 Kind;
		};

		class XYZ_API PremakeGenerator
		{
		public:
			PremakeGenerator(std::filesystem::path projectPath, const PremakeProject& project);

			void Generate();

			PremakeProject& GetProject() { return m_Project; }
		private:
			std::filesystem::path m_ProjectPath;
			PremakeProject m_Project;
		};

	}
}