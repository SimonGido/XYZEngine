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
	
	namespace Script {
		
		struct NativeAPI
		{
			std::function<SpriteRenderer* (uint32_t)> GetSpriteRenderer = &XYZ::GetSpriteRenderer;
			std::function<TransformComponent* (uint32_t)> GetTransform = &XYZ::GetTransformComponent;
		};
	}

}

