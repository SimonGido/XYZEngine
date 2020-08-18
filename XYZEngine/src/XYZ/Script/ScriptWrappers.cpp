#include "stdafx.h"
#include "ScriptWrappers.h"


#include "XYZ/Scene/Scene.h"
#include "XYZ/ECS/Entity.h"
#include "XYZ/Scene/Components.h"

#include <glm/gtc/type_ptr.hpp>


#include "XYZ/Core/Input.h"
#include <mono/jit/jit.h>


namespace XYZ {
	extern std::unordered_map<MonoType*, std::function<bool(Entity&)>> s_HasComponentFuncs;
	extern std::unordered_map<MonoType*, std::function<void(Entity&)>> s_CreateComponentFuncs;
}

namespace XYZ { namespace Script {

	enum class ComponentID
	{
		None = 0,
		Transform = 1,
		Mesh = 2,
		Script = 3,
		SpriteRenderer = 4
	};



	////////////////////////////////////////////////////////////////
	// Math ////////////////////////////////////////////////////////
	////////////////////////////////////////////////////////////////

	

	////////////////////////////////////////////////////////////////

	////////////////////////////////////////////////////////////////
	// Input ///////////////////////////////////////////////////////
	////////////////////////////////////////////////////////////////

	bool XYZ_Input_IsKeyPressed(KeyCode key)
	{
		return Input::IsKeyPressed(key);
	}

	////////////////////////////////////////////////////////////////

	////////////////////////////////////////////////////////////////
	// Entity //////////////////////////////////////////////////////
	////////////////////////////////////////////////////////////////

	void XYZ_Entity_GetTransform(uint32_t entityID, glm::mat4* outTransform)
	{
		Ref<Scene> scene = ScriptEngine::GetCurrentSceneContext();
		XYZ_ASSERT(scene, "No active scene!");
	
		Entity entity = Entity(entityID, scene.Raw());
		auto transformComponent = entity.GetComponent<TransformComponent>();
		memcpy(outTransform, glm::value_ptr(transformComponent->Transform), sizeof(glm::mat4));
	}

	void XYZ_Entity_SetTransform(uint32_t entityID, glm::mat4* inTransform)
	{
		Ref<Scene> scene = ScriptEngine::GetCurrentSceneContext();
		XYZ_ASSERT(scene, "No active scene!");
		Entity entity = Entity(entityID, scene.Raw());
		

		auto transformComponent = entity.GetComponent<TransformComponent>();
		memcpy(glm::value_ptr(transformComponent->Transform), inTransform, sizeof(glm::mat4));
	}

	void XYZ_Entity_CreateComponent(uint32_t entityID, void* type)
	{
		Ref<Scene> scene = ScriptEngine::GetCurrentSceneContext();
		XYZ_ASSERT(scene, "No active scene!");
		Entity entity = Entity(entityID, scene.Raw());

	
		MonoType* monoType = mono_reflection_type_get_type((MonoReflectionType*)type);
		s_CreateComponentFuncs[monoType](entity);
	}

	bool XYZ_Entity_HasComponent(uint32_t entityID, void* type)
	{
		Ref<Scene> scene = ScriptEngine::GetCurrentSceneContext();
		XYZ_ASSERT(scene, "No active scene!");
		Entity entity = Entity(entityID, scene.Raw());

		MonoType* monoType = mono_reflection_type_get_type((MonoReflectionType*)type);
		bool result = s_HasComponentFuncs[monoType](entity);
		return result;
	}

	
	void* XYZ_Texture2D_Constructor(uint32_t width, uint32_t height)
	{
		auto result = Texture2D::Create(TextureFormat::RGBA,TextureWrap::Clamp, width, height);
		return new Ref<Texture2D>(result);
	}

	void XYZ_Texture2D_Destructor(Ref<Texture2D>* _this)
	{
		delete _this;
	}

	void XYZ_Texture2D_SetData(Ref<Texture2D>* _this, MonoArray* inData, int32_t count)
	{
		Ref<Texture2D>& instance = *_this;
		
		uint32_t dataSize = count * sizeof(glm::vec4) / 4;

		size_t size = instance->GetSpecification().Width* instance->GetSpecification().Height;
		
		XYZ_ASSERT(dataSize <= size,"");
		// Convert RGBA32F color to RGBA8
		uint8_t* pixels = instance->GetData();
		uint32_t index = 0;
		for (int i = 0; i < instance->GetWidth() * instance->GetHeight(); i++)
		{
			glm::vec4& value = mono_array_get(inData, glm::vec4, i);
			*pixels++ = (uint32_t)(value.x * 255.0f);
			*pixels++ = (uint32_t)(value.y * 255.0f);
			*pixels++ = (uint32_t)(value.z * 255.0f);
			*pixels++ = (uint32_t)(value.w * 255.0f);
		}
	}

	void XYZ_Material_Destructor(Ref<Material>* _this)
	{
		delete _this;
	}

	void XYZ_Material_SetFloat(Ref<Material>* _this, MonoString* uniform, float value)
	{
		Ref<Material>& instance = *(Ref<Material>*)_this;
		instance->Set(mono_string_to_utf8(uniform), value);
	}

	void XYZ_Material_SetTexture(Ref<Material>* _this, MonoString* uniform, Ref<Texture2D>* texture)
	{
		Ref<Material>& instance = *(Ref<Material>*)_this;
		instance->Set(mono_string_to_utf8(uniform), *texture);
	}

	void XYZ_MaterialInstance_Destructor(Ref<MaterialInstance>* _this)
	{
		delete _this;
	}

	void XYZ_MaterialInstance_SetFloat(Ref<MaterialInstance>* _this, MonoString* uniform, float value)
	{
		Ref<MaterialInstance>& instance = *(Ref<MaterialInstance>*)_this;
		instance->Set(mono_string_to_utf8(uniform), value);
	}

	void XYZ_MaterialInstance_SetVector3(Ref<MaterialInstance>* _this, MonoString* uniform, glm::vec3* value)
	{
		Ref<MaterialInstance>& instance = *(Ref<MaterialInstance>*)_this;
		instance->Set(mono_string_to_utf8(uniform), *value);
	}

	void XYZ_MaterialInstance_SetVector4(Ref<MaterialInstance>* _this, MonoString* uniform, glm::vec4* value)
	{
		Ref<MaterialInstance>& instance = *(Ref<MaterialInstance>*)_this;
		instance->Set(mono_string_to_utf8(uniform), *value);
	}

	void XYZ_MaterialInstance_SetTexture(Ref<MaterialInstance>* _this, MonoString* uniform, Ref<Texture2D>* texture)
	{
		Ref<MaterialInstance>& instance = *(Ref<MaterialInstance>*)_this;
		instance->Set(mono_string_to_utf8(uniform), *texture);
	}


	////////////////////////////////////////////////////////////////

} }