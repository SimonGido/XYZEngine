#pragma once

#include "XYZ/Script/ScriptWrappers.h"
#include "XYZ/Renderer/Texture.h"


namespace XYZ {
	namespace Script {
		struct Texture2DNative
		{
			static void			   Register();
			static Ref<Texture2D>* Constructor(uint32_t width, uint32_t height);
			static Ref<Texture2D>* Constructor_Path(MonoString* path);

			static void			   Destructor(Ref<Texture2D>* instance);
			static void			   SetData(Ref<Texture2D>* instance, MonoArray* inData, int32_t count);
		};
	}
}