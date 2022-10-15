#include "stdafx.h"
#include "SceneEntityNative.h"
#include "XYZ/Scene/SceneEntity.h"
#include "XYZ/Scene/Components.h"

#include "XYZ/Utils/Delegate.h"

#include <mono/jit/jit.h>
#include <mono/metadata/assembly.h>


#define REGISTER_COMPONENT_TYPE(Type) \
	{\
		MonoType* type = mono_reflection_type_from_name("XYZ." #Type, s_CoreAssemblyImage);\
	    XYZ_ASSERT(type != nullptr, "No C# component class found for " #Type "!");\
		SceneEntityFunc entity;\
		\
		entity.HasComponentFunc.Connect<&Utils::HasComponent<Type>>(); \
		entity.EmplaceComponentFunc.Connect<&Utils::EmplaceComponent<Type>>(); \
		entity.RemoveComponentFunc.Connect<&Utils::RemoveComponent<Type>>(); \
		s_NativeEntityFuncs[type] = entity;\
	}


namespace XYZ {
	extern MonoImage* s_CoreAssemblyImage;

	namespace Script {
		namespace Utils {
			template <typename T>
			bool HasComponent(SceneEntity& entity)
			{
				return entity.HasComponent<T>();
			}

			template <typename T>
			void EmplaceComponent(SceneEntity& entity)
			{
				entity.EmplaceComponent<T>();
			}

			template <typename T>
			void RemoveComponent(SceneEntity& entity)
			{
				entity.RemoveComponent<T>();
			}
		}
		struct SceneEntityFunc
		{
			Delegate<bool(SceneEntity&)> HasComponentFunc;
			Delegate<void(SceneEntity&)> EmplaceComponentFunc;
			Delegate<void(SceneEntity&)> RemoveComponentFunc;
		};

		static std::unordered_map<MonoType*, SceneEntityFunc> s_NativeEntityFuncs;
		
		
	
		static bool HasComponent(uint32_t entity, MonoReflectionType* type)
		{
			Ref<Scene> scene = ScriptEngine::GetCurrentSceneContext();
			XYZ_ASSERT(scene.Raw(), "No active scene!");

			SceneEntity ent(static_cast<entt::entity>(entity), scene.Raw());

			MonoType* mType = mono_reflection_type_get_type(type);
			return s_NativeEntityFuncs[mType].HasComponentFunc(ent);
		}
		static void EmplaceComponent(uint32_t entity, MonoReflectionType* type)
		{
			Ref<Scene> scene = ScriptEngine::GetCurrentSceneContext();
			XYZ_ASSERT(scene.Raw(), "No active scene!");

			SceneEntity ent(static_cast<entt::entity>(entity), scene.Raw());

			MonoType* mType = mono_reflection_type_get_type(type);
			s_NativeEntityFuncs[mType].EmplaceComponentFunc(ent);
		}
		static void RemoveComponent(uint32_t entity, MonoReflectionType* type)
		{
			Ref<Scene> scene = ScriptEngine::GetCurrentSceneContext();
			XYZ_ASSERT(scene.Raw(), "No active scene!");

			SceneEntity ent(static_cast<entt::entity>(entity), scene.Raw());

			MonoType* mType = mono_reflection_type_get_type(type);
			s_NativeEntityFuncs[mType].RemoveComponentFunc(ent);
		}
		void SceneEntityNative::Register()
		{
			REGISTER_COMPONENT_TYPE(SceneTagComponent);
			REGISTER_COMPONENT_TYPE(TransformComponent);
			REGISTER_COMPONENT_TYPE(SpriteRenderer);
			REGISTER_COMPONENT_TYPE(ScriptComponent);
			REGISTER_COMPONENT_TYPE(CameraComponent);
			REGISTER_COMPONENT_TYPE(ParticleComponent);
			REGISTER_COMPONENT_TYPE(Relationship);
			REGISTER_COMPONENT_TYPE(PointLightComponent2D);
			REGISTER_COMPONENT_TYPE(RigidBody2DComponent);
			REGISTER_COMPONENT_TYPE(BoxCollider2DComponent);
			REGISTER_COMPONENT_TYPE(CircleCollider2DComponent);
			REGISTER_COMPONENT_TYPE(PolygonCollider2DComponent);
			REGISTER_COMPONENT_TYPE(MeshComponent);
			REGISTER_COMPONENT_TYPE(AnimatedMeshComponent);
			REGISTER_COMPONENT_TYPE(AnimationComponent);

			mono_add_internal_call("XYZ.Entity::HasComponent_Native",    HasComponent);
			mono_add_internal_call("XYZ.Entity::CreateComponent_Native", EmplaceComponent);
			mono_add_internal_call("XYZ.Entity::RemoveComponent_Native", RemoveComponent);
			mono_add_internal_call("XYZ.Entity::Create_Native", Create);
		}

		uint32_t SceneEntityNative::Create()
		{
			Ref<Scene> scene = ScriptEngine::GetCurrentSceneContext();
			XYZ_ASSERT(scene.Raw(), "No active scene!");

			return scene->CreateEntity("", GUID());
		}	
	}
}