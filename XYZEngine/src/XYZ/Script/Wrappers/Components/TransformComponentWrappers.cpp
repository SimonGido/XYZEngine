#include "stdafx.h"
#include "TransformComponentWrappers.h"

#include "XYZ/Scene/SceneEntity.h"
#include "XYZ/Scene/Components.h"

#include "XYZ/Script/ScriptEngine.h"

#include <glm/gtc/type_ptr.hpp>

#include <mono/jit/jit.h>
#include <mono/metadata/assembly.h>

namespace XYZ {
	namespace Script {
		void TransformComponentNative::Register()
		{
			mono_add_internal_call("XYZ.TransformComponent::GetTransform_Native", GetTransform);
			mono_add_internal_call("XYZ.TransformComponent::SetTransform_Native", SetTransform);
			mono_add_internal_call("XYZ.TransformComponent::GetTranslation_Native", GetTranslation);
			mono_add_internal_call("XYZ.TransformComponent::SetTranslation_Native", SetTranslation);
			mono_add_internal_call("XYZ.TransformComponent::GetRotation_Native", GetRotation);
			mono_add_internal_call("XYZ.TransformComponent::SetRotation_Native", SetRotation);
			mono_add_internal_call("XYZ.TransformComponent::GetScale_Native", GetScale);
			mono_add_internal_call("XYZ.TransformComponent::SetScale_Native", SetScale);

		}
		void TransformComponentNative::GetTransform(uint32_t entity, glm::mat4* outTransform)
		{
			Ref<Scene> scene = ScriptEngine::GetCurrentSceneContext();
			XYZ_ASSERT(scene.Raw(), "No active scene!");

			SceneEntity ent(static_cast<entt::entity>(entity), scene.Raw());

			const auto& transformComponent = ent.GetComponent<TransformComponent>();
			memcpy(outTransform, glm::value_ptr(transformComponent.GetLocalTransform()), sizeof(glm::mat4));
		}

		void TransformComponentNative::SetTransform(uint32_t entity, glm::mat4* inTransform)
		{
			Ref<Scene> scene = ScriptEngine::GetCurrentSceneContext();
			XYZ_ASSERT(scene.Raw(), "No active scene!");

			SceneEntity ent(static_cast<entt::entity>(entity), scene.Raw());
			ent.GetComponent<TransformComponent>().DecomposeTransform(*inTransform);
		}

		void TransformComponentNative::GetTranslation(uint32_t entity, glm::vec3* out)
		{
			Ref<Scene> scene = ScriptEngine::GetCurrentSceneContext();
			XYZ_ASSERT(scene.Raw(), "No active scene!");

			SceneEntity ent(static_cast<entt::entity>(entity), scene.Raw());
			memcpy(out, &ent.GetComponent<TransformComponent>()->Translation, sizeof(glm::vec3));
		}

		void TransformComponentNative::SetTranslation(uint32_t entity, glm::vec3* in)
		{
			Ref<Scene> scene = ScriptEngine::GetCurrentSceneContext();
			XYZ_ASSERT(scene.Raw(), "No active scene!");

			SceneEntity ent(static_cast<entt::entity>(entity), scene.Raw());

			ent.GetComponent<TransformComponent>().GetTransform().Translation = *in;
		}

		void TransformComponentNative::GetRotation(uint32_t entity, glm::vec3* out)
		{
			Ref<Scene> scene = ScriptEngine::GetCurrentSceneContext();
			XYZ_ASSERT(scene.Raw(), "No active scene!");

			SceneEntity ent(static_cast<entt::entity>(entity), scene.Raw());
			memcpy(out, &ent.GetComponent<TransformComponent>()->Rotation, sizeof(glm::vec3));
		}

		void TransformComponentNative::SetRotation(uint32_t entity, glm::vec3* in)
		{
			Ref<Scene> scene = ScriptEngine::GetCurrentSceneContext();
			XYZ_ASSERT(scene.Raw(), "No active scene!");

			SceneEntity ent(static_cast<entt::entity>(entity), scene.Raw());

			ent.GetComponent<TransformComponent>().GetTransform().Rotation = *in;
		}

		void TransformComponentNative::GetScale(uint32_t entity, glm::vec3* out)
		{
			Ref<Scene> scene = ScriptEngine::GetCurrentSceneContext();
			XYZ_ASSERT(scene.Raw(), "No active scene!");

			SceneEntity ent(static_cast<entt::entity>(entity), scene.Raw());
			memcpy(out, &ent.GetComponent<TransformComponent>()->Scale, sizeof(glm::vec3));
		}

		void TransformComponentNative::SetScale(uint32_t entity, glm::vec3* in)
		{
			Ref<Scene> scene = ScriptEngine::GetCurrentSceneContext();
			XYZ_ASSERT(scene.Raw(), "No active scene!");

			SceneEntity ent(static_cast<entt::entity>(entity), scene.Raw());

			ent.GetComponent<TransformComponent>().GetTransform().Scale = *in;
		}
	}
}