#pragma once
#include "XYZ/Scene/Components.h"
#include "XYZ/ECS/Entity.h"
#include <glm/glm.hpp>


namespace XYZ {
	enum InterfaceIDEnum
	{
		ID_SCRIPTABLE_ENTITY,
		IID_IRCCPP_MAIN_LOOP
	};

	SpriteRenderer* GetSpriteRenderer(uint32_t entity);
	TransformComponent* GetTransformComponent(uint32_t entity);
	CameraComponent* GetCameraComponent(uint32_t entity);
	SceneTagComponent* GetSceneTagComponent(uint32_t entity);

	namespace Script {
		
		struct NativeAPI
		{
			const std::function<SpriteRenderer* (uint32_t)> GetSpriteRenderer = &XYZ::GetSpriteRenderer;
			const std::function<TransformComponent* (uint32_t)> GetTransform = &XYZ::GetTransformComponent;
			const std::function<CameraComponent* (uint32_t)> GetCamera = &XYZ::GetCameraComponent;
			const std::function<SceneTagComponent* (uint32_t)> GetSceneTag = &XYZ::GetSceneTagComponent;
		};
	}

}

