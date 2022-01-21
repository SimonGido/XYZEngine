#include "stdafx.h"
#include "MaterialWrapper.h"

#include <mono/jit/jit.h>
#include <mono/metadata/assembly.h>

namespace XYZ {
	namespace Script {

	

		void MaterialNative::Register()
		{
			mono_add_internal_call("XYZ.Material::Constructor_Native", Constructor);
			mono_add_internal_call("XYZ.Material::Destructor_Native", Destructor);
			mono_add_internal_call("XYZ.Material.SetFloat_Native", SetFloat);
			mono_add_internal_call("XYZ.Material.SetInt_Native",	SetInt  );
			mono_add_internal_call("XYZ.Material.SetVec2_Native", SetVec2 );
			mono_add_internal_call("XYZ.Material.SetVec3_Native", SetVec3);
			mono_add_internal_call("XYZ.Material.SetVec4_Native", SetVec4);
			mono_add_internal_call("XYZ.Material.SetMat4_Native", SetMat4);
			mono_add_internal_call("XYZ.Material.SetIVec2_Native", SetIVec2);
			mono_add_internal_call("XYZ.Material.SetIVec3_Native", SetIVec3);
			mono_add_internal_call("XYZ.Material.SetIVec4_Native", SetIVec4);
		}
		Ref<Material>* MaterialNative::Constructor(Ref<Shader>* shader)
		{
			Ref<Material> material = Material::Create(*shader);
			return new Ref<Material>(material);
		}
		void MaterialNative::Destructor(Ref<Material>* instance)
		{
			delete instance;
		}
		void MaterialNative::SetFloat(Ref<Material>* instance, MonoString* name, float value)
		{
			Ref<Material>& mat = *instance;
			mat->Set(mono_string_to_utf8(name), value);
		}
		void MaterialNative::SetInt(Ref<Material>* instance, MonoString* name, int value)
		{
			Ref<Material>& mat = *instance;
			mat->Set(mono_string_to_utf8(name), value);
		}
		void MaterialNative::SetVec2(Ref<Material>* instance, MonoString* name, glm::vec2* value)
		{
			Ref<Material>& mat = *instance;
			mat->Set(mono_string_to_utf8(name), *value);
		}
		void MaterialNative::SetVec3(Ref<Material>* instance, MonoString* name, glm::vec3* value)
		{
			Ref<Material>& mat = *instance;
			mat->Set(mono_string_to_utf8(name), *value);
		}
		void MaterialNative::SetVec4(Ref<Material>* instance, MonoString* name, glm::vec4* value)
		{
			Ref<Material>& mat = *instance;
			mat->Set(mono_string_to_utf8(name), *value);
		}
		void MaterialNative::SetMat4(Ref<Material>* instance, MonoString* name, glm::mat4* value)
		{
			Ref<Material>& mat = *instance;
			mat->Set(mono_string_to_utf8(name), *value);
		}
		void MaterialNative::SetIVec2(Ref<Material>* instance, MonoString* name, glm::ivec2* value)
		{
			Ref<Material>& mat = *instance;
			mat->Set(mono_string_to_utf8(name), *value);
		}
		void MaterialNative::SetIVec3(Ref<Material>* instance, MonoString* name, glm::ivec3* value)
		{
			Ref<Material>& mat = *instance;
			mat->Set(mono_string_to_utf8(name), *value);
		}
		void MaterialNative::SetIVec4(Ref<Material>* instance, MonoString* name, glm::ivec4* value)
		{
			Ref<Material>& mat = *instance;
			mat->Set(mono_string_to_utf8(name), *value);
		}
	}
}