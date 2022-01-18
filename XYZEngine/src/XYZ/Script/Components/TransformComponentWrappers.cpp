#include "stdafx.h"
#include "TransformComponentWrappers.h"

#include "XYZ/Scene/SceneEntity.h"
#include "XYZ/Scene/Components.h"

#include "XYZ/Script/ScriptEngine.h"

#include <glm/gtc/type_ptr.hpp>

namespace XYZ {
	namespace Script {
		void XYZ_TransformComponent_GetTransform(uint32_t entity, glm::mat4* outTransform)
		{
			Ref<Scene> scene = ScriptEngine::GetCurrentSceneContext();
			XYZ_ASSERT(scene.Raw(), "No active scene!");

			SceneEntity ent(entity, scene.Raw());

			const auto& transformComponent = ent.GetComponent<TransformComponent>();
			memcpy(outTransform, glm::value_ptr(transformComponent.GetTransform()), sizeof(glm::mat4));
		}

		void XYZ_TransformComponent_SetTransform(uint32_t entity, glm::mat4* inTransform)
		{
			Ref<Scene> scene = ScriptEngine::GetCurrentSceneContext();
			XYZ_ASSERT(scene.Raw(), "No active scene!");

			SceneEntity ent(entity, scene.Raw());
			ent.GetComponent<TransformComponent>().DecomposeTransform(*inTransform);
		}

		void XYZ_TransformComponent_GetTranslation(uint32_t entity, glm::vec3* out)
		{
			Ref<Scene> scene = ScriptEngine::GetCurrentSceneContext();
			XYZ_ASSERT(scene.Raw(), "No active scene!");

			SceneEntity ent(entity, scene.Raw());
			memcpy(out, &ent.GetComponent<TransformComponent>().Translation, sizeof(glm::vec3));
		}

		void XYZ_TransformComponent_SetTranslation(uint32_t entity, glm::vec3* in)
		{
			Ref<Scene> scene = ScriptEngine::GetCurrentSceneContext();
			XYZ_ASSERT(scene.Raw(), "No active scene!");

			SceneEntity ent(entity, scene.Raw());

			ent.GetComponent<TransformComponent>().Translation = *in;
		}

		void XYZ_TransformComponent_GetRotation(uint32_t entity, glm::vec3* out)
		{
			Ref<Scene> scene = ScriptEngine::GetCurrentSceneContext();
			XYZ_ASSERT(scene.Raw(), "No active scene!");

			SceneEntity ent(entity, scene.Raw());
			memcpy(out, &ent.GetComponent<TransformComponent>().Rotation, sizeof(glm::vec3));
		}

		void XYZ_TransformComponent_SetRotation(uint32_t entity, glm::vec3* in)
		{
			Ref<Scene> scene = ScriptEngine::GetCurrentSceneContext();
			XYZ_ASSERT(scene.Raw(), "No active scene!");

			SceneEntity ent(entity, scene.Raw());

			ent.GetComponent<TransformComponent>().Rotation = *in;
		}

		void XYZ_TransformComponent_GetScale(uint32_t entity, glm::vec3* out)
		{
			Ref<Scene> scene = ScriptEngine::GetCurrentSceneContext();
			XYZ_ASSERT(scene.Raw(), "No active scene!");

			SceneEntity ent(entity, scene.Raw());
			memcpy(out, &ent.GetComponent<TransformComponent>().Scale, sizeof(glm::vec3));
		}

		void XYZ_TransformComponent_SetScale(uint32_t entity, glm::vec3* in)
		{
			Ref<Scene> scene = ScriptEngine::GetCurrentSceneContext();
			XYZ_ASSERT(scene.Raw(), "No active scene!");

			SceneEntity ent(entity, scene.Raw());

			ent.GetComponent<TransformComponent>().Scale = *in;
		}
	}
}