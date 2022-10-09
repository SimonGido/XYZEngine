#pragma once

#include <shaderc/shaderc.hpp>

namespace XYZ {

	class ShaderIncluder : shaderc::CompileOptions::IncluderInterface
	{
	public:
		virtual shaderc_include_result* GetInclude(
			const char* requested_source,
			shaderc_include_type type,
			const char* requesting_source,
			size_t include_depth) override;

		// Handles shaderc_include_result_release_fn callbacks.
		virtual void ReleaseInclude(shaderc_include_result* data) override;

		
		static std::unique_ptr<shaderc::CompileOptions::IncluderInterface> Create();
	};

}