#pragma once
#include <glm/glm.hpp>


extern "C"
{
	typedef struct _MonoString MonoString;
	typedef struct _MonoArray MonoArray;
}



namespace XYZ {
	namespace Script {
		struct TransformComponentNative
		{
			static void Register();
		private:
			static void GetTransform(uint32_t entity, glm::mat4* outTransform);
			static void SetTransform(uint32_t entity, glm::mat4* inTransform);
			static void GetTranslation(uint32_t entity, glm::vec3* out);
			static void SetTranslation(uint32_t entity, glm::vec3* in);
			static void GetRotation(uint32_t entity, glm::vec3* out);
			static void SetRotation(uint32_t entity, glm::vec3* in);
			static void GetScale(uint32_t entity, glm::vec3* out);
			static void SetScale(uint32_t entity, glm::vec3* in);
		};
		
	}
}