#pragma once
#include "XYZ/Renderer/SubTexture.h"

namespace XYZ {
	namespace Script {
		struct SubTextureNative
		{
			static void Register();
			static Ref<SubTexture>* Constructor(Ref<Texture2D>* texture);
			static Ref<SubTexture>* Constructor_TexCoords(Ref<Texture2D>* texture, glm::vec4* inTexCoords);
			static void				Destructor(Ref<SubTexture>* instance);
			static Ref<Texture2D>*	GetTexture(Ref<SubTexture>* instance);
			static void				GetTexCoords(Ref<SubTexture>* instance, glm::vec4* outTexCoords);
		};
	}
}