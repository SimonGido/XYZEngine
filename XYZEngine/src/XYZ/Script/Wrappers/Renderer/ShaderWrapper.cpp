#include "stdafx.h"
#include "ShaderWrapper.h"

#include <mono/jit/jit.h>
#include <mono/metadata/assembly.h>

namespace XYZ {
	namespace Script {


		void ShaderNative::Register()
		{
			mono_add_internal_call("XYZ.Shader::Constructor_Native", Constructor);
			mono_add_internal_call("XYZ.Shader::Destructor_Native", Destructor);
		}
		Ref<Shader>* ShaderNative::Constructor(MonoString* path)
		{
			XYZ_ASSERT(false, "Specify buffer layout");
			Ref<Shader> shader = Shader::Create(mono_string_to_utf8(path), {});
			return new Ref<Shader>(shader);
		}
		void ShaderNative::Destructor(Ref<Shader>* instance)
		{
			delete instance;
		}
	}
}