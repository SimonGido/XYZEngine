#pragma once
#include "XYZ/ECS/Component.h"
#include "SceneCamera.h"

namespace XYZ {

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
}