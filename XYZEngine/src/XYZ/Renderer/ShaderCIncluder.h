#pragma once

#include "ShaderIncluder.h"

#include <shaderc/shaderc.hpp>

namespace XYZ {
	class XYZ_API ShaderCIncluder : public shaderc::CompileOptions::IncluderInterface
	{
	public:
		ShaderCIncluder(const ShaderIncluder& includer);
		
		virtual ~ShaderCIncluder() override;
		

		virtual shaderc_include_result* GetInclude(
			const char* requested_source,
			shaderc_include_type type,
			const char* requesting_source,
			size_t include_depth) override;
		

		// Not used, release of includes is handled by ShaderIncluder
		virtual void ReleaseInclude(shaderc_include_result* data) override
		{
		
		}

		static std::unique_ptr<shaderc::CompileOptions::IncluderInterface> Create(const ShaderIncluder& includer);
	private:
		std::unordered_map<std::string, shaderc_include_result*> m_Includes;
	};
}