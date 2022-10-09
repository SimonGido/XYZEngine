#include "stdafx.h"
#include "ShaderIncluder.h"

namespace XYZ {
	shaderc_include_result* ShaderIncluder::GetInclude(const char* requested_source, shaderc_include_type type, const char* requesting_source, size_t include_depth)
	{
		return nullptr;
	}
	void ShaderIncluder::ReleaseInclude(shaderc_include_result* data)
	{
	}
	std::unique_ptr<shaderc::CompileOptions::IncluderInterface> ShaderIncluder::Create()
	{
		return std::unique_ptr<shaderc::CompileOptions::IncluderInterface>(new ShaderIncluder());
	}
}