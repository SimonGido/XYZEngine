#include "stdafx.h"
#include "AnimationComponentWrapper.h"

#include "XYZ/Script/ScriptEngine.h"
#include "XYZ/Scene/SceneEntity.h"
#include "XYZ/Scene/Components.h"
#include "XYZ/Scene/Prefab.h"


#include <mono/jit/jit.h>
#include <mono/metadata/assembly.h>

namespace XYZ {
	namespace Script {
		void AnimationComponentNative::Register()
		{
			mono_add_internal_call("XYZ.AnimationComponent::GetController_Native", GetController);
			mono_add_internal_call("XYZ.AnimationComponent::GetBoneEntities_Native", GetBoneEntities);
			mono_add_internal_call("XYZ.AnimationComponent::GetAnimationTime_Native", GetAnimationTime);
			mono_add_internal_call("XYZ.AnimationComponent::GetPlaying_Native", GetPlaying);

			mono_add_internal_call("XYZ.AnimationComponent::SetController_Native", SetController);
			mono_add_internal_call("XYZ.AnimationComponent::SetPlaying_Native", SetPlaying);
			mono_add_internal_call("XYZ.AnimationComponent::SetAnimationTime_Native", SetAnimationTime);
			mono_add_internal_call("XYZ.AnimationComponent::SetBoneEntities_Native", SetBoneEntities);
		}
		Ref<AnimationController>* AnimationComponentNative::GetController(uint32_t entity)
		{
			Ref<Scene> scene = ScriptEngine::GetCurrentSceneContext();
			XYZ_ASSERT(scene.Raw(), "No active scene!");

			SceneEntity ent(static_cast<entt::entity>(entity), scene.Raw());
			return new Ref<AnimationController>(ent.GetComponent<AnimationComponent>().Controller.Value());
		}
		MonoArray* AnimationComponentNative::GetBoneEntities(uint32_t entity)
		{
			Ref<Scene> scene = ScriptEngine::GetCurrentSceneContext();
			XYZ_ASSERT(scene.Raw(), "No active scene!");

			SceneEntity ent(static_cast<entt::entity>(entity), scene.Raw());
			//auto& boneEntities = ent.GetComponent<AnimationComponent>().BoneEntities;

			//MonoArray* result = mono_array_new(mono_domain_get(), ScriptEngine::GetCoreClass("XYZ.Entity"), boneEntities.size());
			MonoArray* result;
			//uint32_t index = 0;
			//for (auto boneEntity : boneEntities)
			//{
			//	mono_array_set(result, entt::entity, index++, boneEntity);
			//}
			return result;
		}

		float AnimationComponentNative::GetAnimationTime(uint32_t entity)
		{
			Ref<Scene> scene = ScriptEngine::GetCurrentSceneContext();
			XYZ_ASSERT(scene.Raw(), "No active scene!");

			SceneEntity ent(static_cast<entt::entity>(entity), scene.Raw());
			return ent.GetComponent<AnimationComponent>().AnimationTime;
		}
		bool AnimationComponentNative::GetPlaying(uint32_t entity)
		{
			Ref<Scene> scene = ScriptEngine::GetCurrentSceneContext();
			XYZ_ASSERT(scene.Raw(), "No active scene!");

			SceneEntity ent(static_cast<entt::entity>(entity), scene.Raw());
			return ent.GetComponent<AnimationComponent>().Playing;
		}
		void AnimationComponentNative::SetController(uint32_t entity, Ref<AnimationController>* controllerInstance)
		{
			Ref<Scene> scene = ScriptEngine::GetCurrentSceneContext();
			XYZ_ASSERT(scene.Raw(), "No active scene!");

			SceneEntity ent(static_cast<entt::entity>(entity), scene.Raw());
			ent.GetComponent<AnimationComponent>().Controller = *controllerInstance;
		}
		void AnimationComponentNative::SetPlaying(uint32_t entity, bool play)
		{
			Ref<Scene> scene = ScriptEngine::GetCurrentSceneContext();
			XYZ_ASSERT(scene.Raw(), "No active scene!");

			SceneEntity ent(static_cast<entt::entity>(entity), scene.Raw());
			ent.GetComponent<AnimationComponent>().Playing = play;
		}
		void AnimationComponentNative::SetAnimationTime(uint32_t entity, float time)
		{
			Ref<Scene> scene = ScriptEngine::GetCurrentSceneContext();
			XYZ_ASSERT(scene.Raw(), "No active scene!");

			SceneEntity ent(static_cast<entt::entity>(entity), scene.Raw());
			ent.GetComponent<AnimationComponent>().AnimationTime = time;
		}
		void AnimationComponentNative::SetBoneEntities(uint32_t entity, MonoArray* boneEntities)
		{
			Ref<Scene> scene = ScriptEngine::GetCurrentSceneContext();
			XYZ_ASSERT(scene.Raw(), "No active scene!");

			SceneEntity ent(static_cast<entt::entity>(entity), scene.Raw());

			auto& animComponent = ent.GetComponent<AnimationComponent>();
			//animComponent.BoneEntities.clear();
			//
			//auto length = mono_array_length(boneEntities);
			//for (size_t i = 0; i < length; ++i)
			//{
			//	uint32_t ent = (uint32_t)mono_array_get(boneEntities, entt::entity, i);
			//	animComponent.BoneEntities.push_back(static_cast<entt::entity>(ent));
			//}
		}
	}
}