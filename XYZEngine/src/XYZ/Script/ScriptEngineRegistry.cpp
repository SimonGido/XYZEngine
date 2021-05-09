#include "stdafx.h"
#include "ScriptEngineRegistry.h"

#include "XYZ/Scene/SceneEntity.h"
#include "XYZ/Scene/Components.h"
#include "XYZ/Renderer/Animation.h"
#include "ScriptWrappers.h"

#include <mono/jit/jit.h>
#include <mono/metadata/assembly.h>


namespace XYZ {
	
	std::unordered_map<MonoType*, std::function<bool(SceneEntity&)>> s_HasComponentFuncs;
	std::unordered_map<MonoType*, std::function<void(SceneEntity&)>> s_CreateComponentFuncs;

	extern MonoImage* s_CoreAssemblyImage;

#define REGISTER_COMPONENT_TYPE(Type) \
	{\
		MonoType* type = mono_reflection_type_from_name("XYZ." #Type, s_CoreAssemblyImage);\
		if (type) {\
			uint32_t id = mono_type_get_type(type);\
			s_HasComponentFuncs[type] = [](SceneEntity& entity) { return entity.HasComponent<Type>(); };\
			s_CreateComponentFuncs[type] = [](SceneEntity& entity) { entity.AddComponent<Type>(Type()); };\
		} else {\
			XYZ_ASSERT(false, "No C# component class found for " #Type "!");\
		}\
	}

	static void InitComponentTypes()
	{
		REGISTER_COMPONENT_TYPE(SceneTagComponent);
		REGISTER_COMPONENT_TYPE(TransformComponent);
		REGISTER_COMPONENT_TYPE(SpriteRenderer);
		REGISTER_COMPONENT_TYPE(ScriptComponent);
		REGISTER_COMPONENT_TYPE(CameraComponent);
		REGISTER_COMPONENT_TYPE(AnimatorComponent);
		REGISTER_COMPONENT_TYPE(ParticleComponent);
		REGISTER_COMPONENT_TYPE(Relationship);
		REGISTER_COMPONENT_TYPE(PointLight2D);
	}

	void ScriptEngineRegistry::RegisterAll()
	{
		InitComponentTypes();

		mono_add_internal_call("XYZ.Input::IsKeyPressed_Native", XYZ::Script::XYZ_Input_IsKeyPressed);

		mono_add_internal_call("XYZ.Entity::GetTransform_Native", XYZ::Script::XYZ_Entity_GetTransform);
		mono_add_internal_call("XYZ.Entity::SetTransform_Native", XYZ::Script::XYZ_Entity_SetTransform);

	}
}