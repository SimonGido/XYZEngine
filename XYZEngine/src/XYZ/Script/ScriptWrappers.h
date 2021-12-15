#pragma once

#include "ScriptEngine.h"
#include "XYZ/Core/KeyCodes.h"

#include <glm/glm.hpp>

extern "C" 
{
	typedef struct _MonoString MonoString;
	typedef struct _MonoArray MonoArray;
}



namespace XYZ {
	namespace Script {

		// Input
		bool XYZ_Input_IsKeyPressed(KeyCode key);
		
		// Entity
		void XYZ_Entity_GetTransform(uint32_t entity, glm::mat4* outTransform);
		void XYZ_Entity_SetTransform(uint32_t entity, glm::mat4* inTransform);
		
		// RigidBody2D
		void XYZ_RigidBody2D_ApplyForce(uint32_t entity, glm::vec2* impulse, glm::vec2* point);		
	}
}