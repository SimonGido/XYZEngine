#pragma once

#include "XYZ/Script/ScriptWrappers.h"
#include "XYZ/Renderer/Shader.h"



namespace XYZ {
	namespace Script {
		struct ShaderNative
		{
			static void			Register();
			static Ref<Shader>* Constructor(MonoString* path);
			static void			Destructor(Ref<Shader>* instance);
		};
	}
}