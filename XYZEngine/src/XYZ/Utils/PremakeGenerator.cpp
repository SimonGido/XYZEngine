#include "stdafx.h"
#include "PremakeGenerator.h"

namespace XYZ {
	namespace Utils {

		static std::string KindToString(ProjectKind kind)
		{
			switch (kind)
			{
			case XYZ::Utils::ProjectKind::SharedLib:
				return "SharedLib";
			case XYZ::Utils::ProjectKind::StaticLib:
				return "StaticLib";
			case XYZ::Utils::ProjectKind::Executable:
				return "ConsoleApp";
			default:
				break;
			}
			XYZ_ASSERT(false, "");
			return "";
		}

		static std::string CppDialectToString(CppDialect dialect)
		{
			switch (dialect)
			{
			case XYZ::Utils::CppDialect::CPP14:
				return "C++14";
			case XYZ::Utils::CppDialect::CPP17:
				return "C++17";
			case XYZ::Utils::CppDialect::CPP20:
				return "C++20";
			}
			XYZ_ASSERT(false, "");
			return "";
		}

		PremakeGenerator::PremakeGenerator(std::filesystem::path projectPath, const PremakeProject& project)
			:
			m_ProjectPath(std::move(projectPath)),
			m_Project(project)
		{
		}
		void PremakeGenerator::Generate()
		{
			std::string result;
			result += fmt::format("project \"{}\"\n", m_Project.Name);
			result += fmt::format("\tkind \"{}\"\n", KindToString(m_Project.Kind));
			result += fmt::format("\tlanguage \"C++\"\n");
			result += fmt::format("\tcppdialect \"{}\"\n", CppDialectToString(m_Project.Dialect));
			result += fmt::format("\tstaticruntime \"off\"\n");

			result += fmt::format("\ttargetdir\"{}\"\n", m_Project.TargetPath);
			result += fmt::format("\tobjdir\"{}\"\n", m_Project.ObjPath);

			result += "files\n";
			result += "{\n";
			result += "\"**.h\"";
			result += "\"**.cpp\"";


		}
	}
}