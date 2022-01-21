#pragma once
#include "XYZ/Renderer/Material.h"

namespace XYZ {
	namespace Script {
		struct MaterialNative
		{
			static void			  Register();
			static Ref<Material>* Constructor(Ref<Shader>* shader);
			static void			  Destructor(Ref<Material>* instance);


			static void SetFloat(Ref<Material>* instance, MonoString* name, float value);
			static void SetInt(  Ref<Material>* instance, MonoString* name, int value);
			static void SetVec2( Ref<Material>* instance, MonoString* name,  glm::vec2* value);
			static void SetVec3( Ref<Material>* instance, MonoString* name,  glm::vec3* value);
			static void SetVec4( Ref<Material>* instance, MonoString* name,  glm::vec4* value);
			static void SetMat4( Ref<Material>* instance, MonoString* name,  glm::mat4* value);
			static void SetIVec2(Ref<Material>* instance, MonoString* name, glm::ivec2* value);
			static void SetIVec3(Ref<Material>* instance, MonoString* name, glm::ivec3* value);
			static void SetIVec4(Ref<Material>* instance, MonoString* name, glm::ivec4* value);
		};
	}
}