#pragma once
#include "XYZ/ECS/Component.h"
#include "XYZ/Particle/ParticleEffect2D.h"
#include "SceneCamera.h"
#include "AnimationController.h"

#include <glm/glm.hpp>
#include <glm/gtx/transform.hpp>

namespace XYZ {
	class ScriptableEntity;

	class TransformComponent : public Type<TransformComponent>
	{
	public:
		TransformComponent() = default;
		
		TransformComponent(const glm::vec3& translation)
			: Translation(translation)
		{}
		
		glm::vec3 Translation = { 0.0f,0.0f,0.0f };
		glm::vec3 Rotation = { 0.0f,0.0f,0.0f };
		glm::vec3 Scale = { 1.0f,1.0f,1.0f };

		glm::mat4 GetTransform() const
		{
			glm::mat4 rotation = glm::rotate(glm::mat4(1.0f), Rotation.x, { 1, 0, 0 })
							   * glm::rotate(glm::mat4(1.0f), Rotation.y, { 0, 1, 0 })
							   * glm::rotate(glm::mat4(1.0f), Rotation.z, { 0, 0, 1 });

			return glm::translate(glm::mat4(1.0f), Translation)
				 * rotation
				 * glm::scale(glm::mat4(1.0f), Scale);
		}
	};

	struct SceneTagComponent : public Type<SceneTagComponent>
	{
		std::string Name;
		SceneTagComponent(const std::string& name)
			: Name(name)
		{}
		SceneTagComponent(const SceneTagComponent& other)
			: Name(other.Name)
		{}

		operator std::string& () { return Name; }
		operator const  std::string& () const { return Name; }
	};

	struct CameraComponent : public Type<CameraComponent>
	{
		SceneCamera Camera;
		CameraComponent() = default;
	};


	struct NativeScriptComponent : public Type<NativeScriptComponent>
	{
		NativeScriptComponent() = default;
		NativeScriptComponent(ScriptableEntity* scriptableEntity, const std::string& scriptObjectName)
			:
			ScriptableEntity(scriptableEntity),
			ScriptObjectName(scriptObjectName)
		{}

		ScriptableEntity* ScriptableEntity = nullptr;
		std::string ScriptObjectName;
	};

	struct AnimatorComponent : public Type<AnimatorComponent>
	{
		AnimatorComponent() = default;
		
		Ref<AnimationController> Controller;
	};


	struct ParticleComponent : public Type<ParticleComponent>
	{
		ParticleComponent() = default;
		Ref<Material> RenderMaterial;
		Ref<Material> ComputeMaterial;

		Ref<ParticleEffect2D> ParticleEffect;
	};

}