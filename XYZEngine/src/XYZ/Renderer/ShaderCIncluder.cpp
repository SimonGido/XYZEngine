#include "stdafx.h"
#include "ShaderCIncluder.h"

namespace XYZ {
	ShaderCIncluder::ShaderCIncluder(const ShaderIncluder& includer)
	{
		for (auto& [name, content] : includer.GetIncludes())
		{
			shaderc_include_result* include = new shaderc_include_result();
			include->source_name = name.c_str();
			include->source_name_length = name.size();
			include->content = content.c_str();
			include->content_length = content.size();
			m_Includes[name] = include;
		}
	}
	ShaderCIncluder::~ShaderCIncluder()
	{
		for (auto& [name, include] : m_Includes)
			delete include;
	}
	shaderc_include_result* ShaderCIncluder::GetInclude(const char* requested_source, shaderc_include_type type, const char* requesting_source, size_t include_depth)
	{
		auto it = m_Includes.find(requested_source);
		XYZ_ASSERT(it != m_Includes.end(), "Include does not exist");

		return it->second;
	}
	std::unique_ptr<shaderc::CompileOptions::IncluderInterface> ShaderCIncluder::Create(const ShaderIncluder& includer)
	{
		return std::make_unique<ShaderCIncluder>(includer);
	}
}