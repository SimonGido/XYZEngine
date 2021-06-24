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
		REGISTER_COMPONENT_TYPE(ParticleComponentGPU);
		REGISTER_COMPONENT_TYPE(Relationship);
		REGISTER_COMPONENT_TYPE(PointLight2D);
		REGISTER_COMPONENT_TYPE(RigidBody2DComponent);
		REGISTER_COMPONENT_TYPE(BoxCollider2DComponent);
		REGISTER_COMPONENT_TYPE(CircleCollider2DComponent);
		REGISTER_COMPONENT_TYPE(PolygonCollider2DComponent);
	}


	namespace Script {
		static bool XYZ_Entity_HasComponent(uint32_t entity, MonoReflectionType* type)
		{
			Ref<Scene> scene = ScriptEngine::GetCurrentSceneContext();
			XYZ_ASSERT(scene.Raw(), "No active scene!");

			SceneEntity ent(entity, scene.Raw());

			MonoType* mType = mono_reflection_type_get_type(type);
			return s_HasComponentFuncs[mType](ent);
		}

		static void XYZ_Entity_CreateComponent(uint32_t entity, MonoReflectionType* type)
		{
			Ref<Scene> scene = ScriptEngine::GetCurrentSceneContext();
			XYZ_ASSERT(scene.Raw(), "No active scene!");

			SceneEntity ent(entity, scene.Raw());

			MonoType* mType = mono_reflection_type_get_type(type);
			s_CreateComponentFuncs[mType](ent);
		}
	}
	void ScriptEngineRegistry::RegisterAll()
	{
		InitComponentTypes();

		mono_add_internal_call("XYZ.Input::IsKeyPressed_Native", XYZ::Script::XYZ_Input_IsKeyPressed);

		mono_add_internal_call("XYZ.Entity::GetTransform_Native", XYZ::Script::XYZ_Entity_GetTransform);
		mono_add_internal_call("XYZ.Entity::SetTransform_Native", XYZ::Script::XYZ_Entity_SetTransform);
		
		mono_add_internal_call("XYZ.Entity::HasComponent_Native", XYZ::Script::XYZ_Entity_HasComponent);
		mono_add_internal_call("XYZ.Entity::CreateComponent_Native", XYZ::Script::XYZ_Entity_CreateComponent);
		
		mono_add_internal_call("XYZ.RigidBody2DComponent::ApplyForce_Native", XYZ::Script::XYZ_RigidBody2D_ApplyForce);
	}
}