#include "stdafx.h"
#include "ShaderAsset.h"

#include "XYZ/Utils/FileSystem.h"

namespace XYZ {
	ShaderAsset::ShaderAsset(Ref<Shader> shader)
		:
		m_Shader(shader)
	{
	}
	ShaderAsset::ShaderAsset(const std::string& filepath, size_t sourceHash)
	{
		std::string source = FileSystem::ReadFile(filepath);
		size_t newSourceHash = std::hash<std::string>{}(source);
		bool forceCompile = newSourceHash != sourceHash;

		m_Shader = Shader::Create(filepath,forceCompile);
	}
	ShaderAsset::ShaderAsset(const std::string& name, const std::string& filepath, size_t sourceHash)
	{
		std::string source = FileSystem::ReadFile(filepath);
		size_t newSourceHash = std::hash<std::string>{}(source);
		bool forceCompile = newSourceHash != sourceHash;

		m_Shader = Shader::Create(name, filepath, forceCompile);
	}
	ShaderAsset::~ShaderAsset()
	{
	}
	size_t ShaderAsset::GetSourceHash() const
	{
		if (m_Shader.Raw())
		{
			return std::hash<std::string>{}(m_Shader->GetSource());
		}
		return 0;
	}
}