#include "stdafx.h"
#include "ShaderReflection.h"

#include <spirv_cross/spirv_cross.hpp>
#include <spirv_cross/spirv_glsl.hpp>

namespace XYZ {
	ShaderReflection::ShaderReflection(const Ref<Shader>& shader)
	{
		if (shader->IsCompute())
			reflectComputeStage(shader);
	}

	void ShaderReflection::reflectComputeStage(const Ref<Shader>& shader)
	{
		const spirv_cross::Compiler compiler(shader->GetShaderData(ShaderType::Compute));
		spirv_cross::ShaderResources resources = compiler.get_shader_resources();

	}
}