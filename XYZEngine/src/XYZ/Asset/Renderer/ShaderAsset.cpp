#include "stdafx.h"
#include "ShaderAsset.h"

#include "XYZ/Utils/FileSystem.h"

namespace XYZ {
	ShaderAsset::ShaderAsset(Ref<Shader> shader)
		:
		m_Shader(shader),
		m_Layouts(shader->GetLayouts())
	{
		m_SourceHash = std::hash<std::string>{}(shader->GetSource());
	}
	ShaderAsset::ShaderAsset(const std::string& filepath, size_t sourceHash, std::vector<BufferLayout> layouts)
		:
		m_Layouts(std::move(layouts))
	{
		std::string source = FileSystem::ReadFile(filepath);
		size_t newSourceHash = std::hash<std::string>{}(source);
		bool forceCompile = newSourceHash != sourceHash;

		m_Shader = Shader::Create(filepath, m_Layouts, forceCompile);
		m_SourceHash = newSourceHash;
	}
	ShaderAsset::ShaderAsset(const std::string& name, const std::string& filepath, size_t sourceHash, std::vector<BufferLayout> layouts)
		:
		m_Layouts(std::move(layouts))
	{
		std::string source = FileSystem::ReadFile(filepath);
		size_t newSourceHash = std::hash<std::string>{}(source);
		bool forceCompile = newSourceHash != sourceHash;

		m_Shader = Shader::Create(name, filepath, m_Layouts, forceCompile);
		m_SourceHash = newSourceHash;
	}
	void ShaderAsset::SetLayouts(std::vector<BufferLayout> layouts)
	{
		XYZ_ASSERT(!IsFlagSet(AssetFlag::ReadOnly), "Asset is readonly");
		m_Layouts = std::move(layouts);
		m_Shader->SetLayouts(m_Layouts);
	}
}