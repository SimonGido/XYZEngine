#pragma once
#include "Shader.h"

namespace XYZ {
	class XYZ_API ShaderReflection
	{
	public:


	public:
		ShaderReflection(const Ref<Shader>& shader);


	private:
		void reflectComputeStage(const Ref<Shader>& shader);

	private:
	};
}