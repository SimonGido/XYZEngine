#pragma once
#include "XYZ/ECS/Component.h"
#include "SceneCamera.h"
#include "AnimationController.h"

#include <glm/glm.hpp>


namespace XYZ {
	class ScriptableEntity;

	class TransformComponent : public Type<TransformComponent>
	{
	public:
		TransformComponent() = default;
		TransformComponent(const glm::mat4& other)
			: Transform(other)
		{}
		TransformComponent(const TransformComponent& other)
			: Transform(other.Transform)
		{}

		TransformComponent(TransformComponent&& other) noexcept
			: Transform(other.Transform)
		{}

		TransformComponent& operator=(const TransformComponent& other)
		{
			Transform = other.Transform;
			return *this;
		}

		operator glm::mat4& () { return Transform; }
		operator const glm::mat4& () const { return Transform; }

		glm::mat4 Transform = glm::mat4(1);
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


	
}