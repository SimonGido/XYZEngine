#include "stdafx.h"
#include "ShaderAsset.h"

#include "XYZ/Utils/FileSystem.h"

namespace XYZ {
	ShaderAsset::ShaderAsset(Ref<Shader> shader)
		:
		m_Shader(shader)
	{
		m_SourceHash = std::hash<std::string>{}(shader->GetSource());
	}
	ShaderAsset::ShaderAsset(const std::string& filepath, size_t sourceHash)
	{
		std::string source = FileSystem::ReadFile(filepath);
		size_t newSourceHash = std::hash<std::string>{}(source);
		bool forceCompile = newSourceHash != sourceHash;

		m_Shader = Shader::Create(filepath,forceCompile);
		m_SourceHash = newSourceHash;
	}
	ShaderAsset::ShaderAsset(const std::string& name, const std::string& filepath, size_t sourceHash)
	{
		std::string source = FileSystem::ReadFile(filepath);
		size_t newSourceHash = std::hash<std::string>{}(source);
		bool forceCompile = newSourceHash != sourceHash;

		m_Shader = Shader::Create(name, filepath, forceCompile);
		m_SourceHash = newSourceHash;
	}
	ShaderAsset::~ShaderAsset()
	{
	}
}