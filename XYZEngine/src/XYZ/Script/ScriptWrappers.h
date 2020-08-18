#pragma once

#include "XYZ/Script/ScriptEngine.h"
#include "XYZ/Core/KeyCodes.h"

#include <glm/glm.hpp>

extern "C" {
	typedef struct _MonoString MonoString;
	typedef struct _MonoArray MonoArray;
}

namespace XYZ { namespace Script {


	// Input
	bool XYZ_Input_IsKeyPressed(KeyCode key);

	// Entity
	void XYZ_Entity_GetTransform(uint32_t entityID, glm::mat4* outTransform);
	void XYZ_Entity_SetTransform(uint32_t entityID, glm::mat4* inTransform);
	void XYZ_Entity_CreateComponent(uint32_t entityID, void* type);
	bool XYZ_Entity_HasComponent(uint32_t entityID, void* type);

	// Renderer
	// Texture2D
	void* XYZ_Texture2D_Constructor(uint32_t width, uint32_t height);
	void XYZ_Texture2D_Destructor(Ref<Texture2D>* _this);
	void XYZ_Texture2D_SetData(Ref<Texture2D>* _this, MonoArray* inData, int32_t count);

	// Material
	void XYZ_Material_Destructor(Ref<Material>* _this);
	void XYZ_Material_SetFloat(Ref<Material>* _this, MonoString* uniform, float value);
	void XYZ_Material_SetTexture(Ref<Material>* _this, MonoString* uniform, Ref<Texture2D>* texture);

	void XYZ_MaterialInstance_Destructor(Ref<MaterialInstance>* _this);
	void XYZ_MaterialInstance_SetFloat(Ref<MaterialInstance>* _this, MonoString* uniform, float value);
	void XYZ_MaterialInstance_SetVector3(Ref<MaterialInstance>* _this, MonoString* uniform, glm::vec3* value);
	void XYZ_MaterialInstance_SetVector4(Ref<MaterialInstance>* _this, MonoString* uniform, glm::vec4* value);
	void XYZ_MaterialInstance_SetTexture(Ref<MaterialInstance>* _this, MonoString* uniform, Ref<Texture2D>* texture);
} }