#include "stdafx.h"
#include "ScriptWrappers.h"

#include "XYZ/Core/Input.h"
#include "XYZ/Scene/SceneEntity.h"

#include <mono/jit/jit.h>
#include <glm/gtc/type_ptr.hpp>


namespace XYZ {
	namespace Script {

		bool XYZ_Input_IsKeyPressed(KeyCode key)
		{
			return Input::IsKeyPressed(key);
		}

		void XYZ_Entity_GetTransform(uint32_t entity, glm::mat4* outTransform)
		{
			Ref<Scene> scene = ScriptEngine::GetCurrentSceneContext();
			XYZ_ASSERT(scene, "No active scene!");
			
			SceneEntity ent(entity, scene.Raw());
		
			auto& transformComponent = ent.GetComponent<TransformComponent>();
			memcpy(outTransform, glm::value_ptr(transformComponent.Transform), sizeof(glm::mat4));
		}

		void XYZ_Entity_SetTransform(uint32_t entity, glm::mat4* inTransform)
		{
			Ref<Scene> scene = ScriptEngine::GetCurrentSceneContext();
			XYZ_ASSERT(scene, "No active scene!");

			SceneEntity ent(entity, scene.Raw());

			auto& transformComponent = ent.GetComponent<TransformComponent>();
			memcpy(glm::value_ptr(transformComponent.Transform), inTransform, sizeof(glm::mat4));
		}
	}
}