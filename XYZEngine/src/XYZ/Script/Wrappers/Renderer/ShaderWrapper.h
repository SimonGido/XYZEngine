#pragma once

#include "XYZ/Script/ScriptWrappers.h"
#include "XYZ/Renderer/Shader.h"
#include "XYZ/Asset/Renderer/ShaderAsset.h"


namespace XYZ {
	namespace Script {
		struct ShaderNative
		{
			static void			Register();
			static Ref<Shader>* Constructor(MonoString* path, MonoArray* layout);
			static void			Destructor(Ref<Shader>* instance);
		};


		struct ShaderAssetNative
		{
			static void				 Register();
			static Ref<ShaderAsset>* Constructor(Ref<Shader>* shader);
			static void				 Destructor(Ref<ShaderAsset>* instance);
		};
	}
}