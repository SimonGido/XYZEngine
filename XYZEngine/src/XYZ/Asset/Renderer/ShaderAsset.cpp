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
		m_Shader = Shader::Create(filepath, sourceHash, false);
	}
	ShaderAsset::ShaderAsset(const std::string& name, const std::string& filepath, size_t sourceHash)
	{
		m_Shader = Shader::Create(name, filepath, sourceHash, false);
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