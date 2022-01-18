#pragma once
#include <glm/glm.hpp>


extern "C"
{
	typedef struct _MonoString MonoString;
	typedef struct _MonoArray MonoArray;
}



namespace XYZ {
	namespace Script {

		void XYZ_TransformComponent_GetTransform(uint32_t entity, glm::mat4* outTransform);
		void XYZ_TransformComponent_SetTransform(uint32_t entity, glm::mat4* inTransform);
		void XYZ_TransformComponent_GetTranslation(uint32_t entity, glm::vec3* out);
		void XYZ_TransformComponent_SetTranslation(uint32_t entity, glm::vec3* in);
		void XYZ_TransformComponent_GetRotation(uint32_t entity, glm::vec3* out);
		void XYZ_TransformComponent_SetRotation(uint32_t entity, glm::vec3* in);
		void XYZ_TransformComponent_GetScale(uint32_t entity, glm::vec3* out);
		void XYZ_TransformComponent_SetScale(uint32_t entity, glm::vec3* in);
	}
}