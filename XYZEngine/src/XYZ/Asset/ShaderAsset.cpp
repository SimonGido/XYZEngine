#include "stdafx.h"
#include "ShaderAsset.h"


namespace XYZ {
	ShaderAsset::ShaderAsset(const std::string& filepath, std::vector<BufferLayout> layouts)
		:
		m_Layouts(std::move(layouts))
	{
		m_Shader = Shader::Create(filepath, m_Layouts);
	}
	void ShaderAsset::SetLayouts(std::vector<BufferLayout> layouts)
	{
		m_Layouts = std::move(layouts);
		m_Shader->SetLayouts(m_Layouts);
	}
}