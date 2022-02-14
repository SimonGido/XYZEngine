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
		}
		Ref<Material>* MaterialNative::Constructor(Ref<Shader>* shader)
		{
			Ref<Material> mat = Material::Create(*shader);
			return new Ref<Material>(mat);
		}
		void MaterialNative::Destructor(Ref<Material>* instance)
		{
			delete instance;
		}

		void MaterialInstanceNative::Register()
		{
			mono_add_internal_call("XYZ.MaterialInstance::Constructor_Native", Constructor);
			mono_add_internal_call("XYZ.MaterialInstance::Destructor_Native", Destructor);
			mono_add_internal_call("XYZ.MaterialInstance.SetFloat_Native", SetFloat);
			mono_add_internal_call("XYZ.MaterialInstance.SetInt_Native",	SetInt  );
			mono_add_internal_call("XYZ.MaterialInstance.SetVec2_Native", SetVec2 );
			mono_add_internal_call("XYZ.MaterialInstance.SetVec3_Native", SetVec3);
			mono_add_internal_call("XYZ.MaterialInstance.SetVec4_Native", SetVec4);
			mono_add_internal_call("XYZ.MaterialInstance.SetMat4_Native", SetMat4);
			mono_add_internal_call("XYZ.MaterialInstance.SetIVec2_Native", SetIVec2);
			mono_add_internal_call("XYZ.MaterialInstance.SetIVec3_Native", SetIVec3);
			mono_add_internal_call("XYZ.MaterialInstance.SetIVec4_Native", SetIVec4);
		}
		Ref<MaterialInstance>* MaterialInstanceNative::Constructor(Ref<Material>* material)
		{
			Ref<MaterialInstance> materialInstance = Ref<MaterialInstance>::Create(*material);
			return new Ref<MaterialInstance>(materialInstance);
		}
		void MaterialInstanceNative::Destructor(Ref<MaterialInstance>* instance)
		{
			delete instance;
		}
		void MaterialInstanceNative::SetFloat(Ref<MaterialInstance>* instance, MonoString* name, float value)
		{
			Ref<MaterialInstance>& mat = *instance;
			mat->Set(mono_string_to_utf8(name), value);
		}
		void MaterialInstanceNative::SetInt(Ref<MaterialInstance>* instance, MonoString* name, int value)
		{
			Ref<MaterialInstance>& mat = *instance;
			mat->Set(mono_string_to_utf8(name), value);
		}
		void MaterialInstanceNative::SetVec2(Ref<MaterialInstance>* instance, MonoString* name, glm::vec2* value)
		{
			Ref<MaterialInstance>& mat = *instance;
			mat->Set(mono_string_to_utf8(name), *value);
		}
		void MaterialInstanceNative::SetVec3(Ref<MaterialInstance>* instance, MonoString* name, glm::vec3* value)
		{
			Ref<MaterialInstance>& mat = *instance;
			mat->Set(mono_string_to_utf8(name), *value);
		}
		void MaterialInstanceNative::SetVec4(Ref<MaterialInstance>* instance, MonoString* name, glm::vec4* value)
		{
			Ref<MaterialInstance>& mat = *instance;
			mat->Set(mono_string_to_utf8(name), *value);
		}
		void MaterialInstanceNative::SetMat4(Ref<MaterialInstance>* instance, MonoString* name, glm::mat4* value)
		{
			Ref<MaterialInstance>& mat = *instance;
			mat->Set(mono_string_to_utf8(name), *value);
		}
		void MaterialInstanceNative::SetIVec2(Ref<MaterialInstance>* instance, MonoString* name, glm::ivec2* value)
		{
			Ref<MaterialInstance>& mat = *instance;
			mat->Set(mono_string_to_utf8(name), *value);
		}
		void MaterialInstanceNative::SetIVec3(Ref<MaterialInstance>* instance, MonoString* name, glm::ivec3* value)
		{
			Ref<MaterialInstance>& mat = *instance;
			mat->Set(mono_string_to_utf8(name), *value);
		}
		void MaterialInstanceNative::SetIVec4(Ref<MaterialInstance>* instance, MonoString* name, glm::ivec4* value)
		{
			Ref<MaterialInstance>& mat = *instance;
			mat->Set(mono_string_to_utf8(name), *value);
		}
		
	}
}