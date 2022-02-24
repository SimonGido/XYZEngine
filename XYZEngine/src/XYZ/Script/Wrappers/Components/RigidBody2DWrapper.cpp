#include "stdafx.h"
#include "RigidBody2DWrapper.h"

#include "XYZ/Script/ScriptEngine.h"
#include "XYZ/Scene/SceneEntity.h"
#include "XYZ/Scene/Components.h"


#include <mono/jit/jit.h>
#include <mono/metadata/assembly.h>

namespace XYZ {
	namespace Script {
		void RigidBody2DNative::Register()
		{
			mono_add_internal_call("XYZ.RigidBody2DComponent::ApplyForce_Native", ApplyForce);

		}
		void RigidBody2DNative::ApplyForce(uint32_t entity, glm::vec2* impulse, glm::vec2* point)
		{
			Ref<Scene> scene = ScriptEngine::GetCurrentSceneContext();
			XYZ_ASSERT(scene.Raw(), "No active scene!");

			SceneEntity ent(static_cast<entt::entity>(entity), scene.Raw());
			const RigidBody2DComponent& rigidBody = ent.GetComponent<RigidBody2DComponent>();
			static_cast<b2Body*>(rigidBody.RuntimeBody)->ApplyLinearImpulse({ impulse->x, impulse->y }, { point->x, point->y }, true);
		}
	}
}