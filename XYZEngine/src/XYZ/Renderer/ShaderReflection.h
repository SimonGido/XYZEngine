#pragma once
#include "Shader.h"

namespace XYZ {
	class ShaderReflection
	{
	public:


	public:
		ShaderReflection(const Ref<Shader>& shader);


	private:
		void reflectComputeStage(const Ref<Shader>& shader);

	private:
	};
}