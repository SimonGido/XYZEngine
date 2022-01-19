#include "stdafx.h"
#include "ScriptEngineRegistry.h"

#include "XYZ/Scene/SceneEntity.h"
#include "XYZ/Scene/Components.h"
#include "XYZ/Animation/Animation.h"
#include "XYZ/Animation/Animator.h"


#include "ScriptWrappers.h"
#include "Components/TransformComponentWrappers.h"
#include "XYZ/Utils/Delegate.h"

#include <mono/jit/jit.h>
#include <mono/metadata/assembly.h>


namespace XYZ {	
	namespace Utils {
		template <typename T>
		bool HasComponent(SceneEntity& entity)
		{
			return entity.HasComponent<T>();
		}

		template <typename T>
		void CreateComponent(SceneEntity& entity)
		{
			entity.EmplaceComponent<T>();
		}
	}


	std::unordered_map<MonoType*, Delegate<bool(SceneEntity&)>> s_HasComponentFuncs;
	std::unordered_map<MonoType*, Delegate<void(SceneEntity&)>> s_CreateComponentFuncs;
	extern MonoImage* s_CoreAssemblyImage;


#define REGISTER_COMPONENT_TYPE(Type) \
	{\
		MonoType* type = mono_reflection_type_from_name("XYZ." #Type, s_CoreAssemblyImage);\
	    XYZ_ASSERT(type != nullptr, "No C# component class found for " #Type "!");\
		Delegate<bool(SceneEntity&)> hasComponentDeleg;\
		Delegate<void(SceneEntity&)> createComponentDeleg;\
		hasComponentDeleg.Connect<&Utils::HasComponent<Type>>();\
		createComponentDeleg.Connect<&Utils::CreateComponent<Type>>();\
		s_HasComponentFuncs[type] = hasComponentDeleg;\
		s_CreateComponentFuncs[type] = createComponentDeleg;\
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

		mono_add_internal_call("XYZ.Entity::HasComponent_Native", XYZ::Script::XYZ_Entity_HasComponent);
		mono_add_internal_call("XYZ.Entity::CreateComponent_Native", XYZ::Script::XYZ_Entity_CreateComponent);

		// Transform component
		mono_add_internal_call("XYZ.TransformComponent::GetTransform_Native", XYZ::Script::XYZ_TransformComponent_GetTransform);
		mono_add_internal_call("XYZ.TransformComponent::SetTransform_Native", XYZ::Script::XYZ_TransformComponent_SetTransform);
		mono_add_internal_call("XYZ.TransformComponent::GetTranslation_Native", XYZ::Script::XYZ_TransformComponent_GetTranslation);
		mono_add_internal_call("XYZ.TransformComponent::SetTranslation_Native", XYZ::Script::XYZ_TransformComponent_SetTranslation);
		mono_add_internal_call("XYZ.TransformComponent::GetRotation_Native", XYZ::Script::XYZ_TransformComponent_GetRotation);
		mono_add_internal_call("XYZ.TransformComponent::SetRotation_Native", XYZ::Script::XYZ_TransformComponent_SetRotation);
		mono_add_internal_call("XYZ.TransformComponent::GetScale_Native", XYZ::Script::XYZ_TransformComponent_GetScale);
		mono_add_internal_call("XYZ.TransformComponent::SetScale_Native", XYZ::Script::XYZ_TransformComponent_SetScale);
		////////////////////////

		mono_add_internal_call("XYZ.RigidBody2DComponent::ApplyForce_Native", XYZ::Script::XYZ_RigidBody2D_ApplyForce);
	}
}