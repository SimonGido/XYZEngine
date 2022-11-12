#include "stdafx.h"
#include "SubTextureWrapper.h"
#include "XYZ/Scene/Prefab.h"


#include <mono/jit/jit.h>
#include <mono/metadata/assembly.h>

namespace XYZ {
	namespace Script {
		void SubTextureNative::Register()
		{
			mono_add_internal_call("XYZ.SubTexture::Constructor_Native", Constructor);
			mono_add_internal_call("XYZ.SubTexture::ConstructorTexCoords_Native", Constructor_TexCoords);
			mono_add_internal_call("XYZ.SubTexture::Destructor_Native", Destructor);
			mono_add_internal_call("XYZ.SubTexture::GetTexture_Native", GetTexture);
			mono_add_internal_call("XYZ.SubTexture::GetTexCoords_Native", GetTexCoords);
		}
		Ref<SubTexture>* SubTextureNative::Constructor(Ref<Texture2D>* texture)
		{
			Ref<SubTexture> subTexture = Ref<SubTexture>::Create(*texture);
			return new Ref<SubTexture>(subTexture);
		}
		Ref<SubTexture>* SubTextureNative::Constructor_TexCoords(Ref<Texture2D>* texture, glm::vec4* inTexCoords)
		{
			Ref<SubTexture> subTexture = Ref<SubTexture>::Create(*texture, *inTexCoords);
			return new Ref<SubTexture>(subTexture);
		}
		void SubTextureNative::Destructor(Ref<SubTexture>* instance)
		{
			delete instance;
		}
		Ref<Texture2D>* SubTextureNative::GetTexture(Ref<SubTexture>* instance)
		{
			Ref<SubTexture>& subTexture = *instance;
			return new Ref<Texture2D>(subTexture->GetTexture().As<Texture2D>());
		}
		void SubTextureNative::GetTexCoords(Ref<SubTexture>* instance, glm::vec4* outTexCoords)
		{
			Ref<SubTexture>& subTexture = *instance;
			*outTexCoords = subTexture->GetTexCoords();
		}
	}
}