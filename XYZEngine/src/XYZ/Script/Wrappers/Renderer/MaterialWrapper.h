#pragma once
#include "XYZ/Renderer/Material.h"
#include "XYZ/Asset/Renderer/MaterialAsset.h"

#include "XYZ/Script/ScriptWrappers.h"

namespace XYZ {
	namespace Script {
		
		struct MaterialNative
		{
			static void			  Register();
			static Ref<Material>* Constructor(Ref<Shader>* shader);
			static void			  Destructor(Ref<Material>* instance);

		};
		struct MaterialInstanceNative
		{
			static void					  Register();
			static Ref<MaterialInstance>* Constructor(Ref<Material>* material);
			static void					  Destructor(Ref<MaterialInstance>* instance);


			static void SetFloat(Ref<MaterialInstance>* instance, MonoString* name, float value);
			static void SetInt(  Ref<MaterialInstance>* instance, MonoString* name, int value);
			static void SetVec2( Ref<MaterialInstance>* instance, MonoString* name,  glm::vec2* value);
			static void SetVec3( Ref<MaterialInstance>* instance, MonoString* name,  glm::vec3* value);
			static void SetVec4( Ref<MaterialInstance>* instance, MonoString* name,  glm::vec4* value);
			static void SetMat4( Ref<MaterialInstance>* instance, MonoString* name,  glm::mat4* value);
			static void SetIVec2(Ref<MaterialInstance>* instance, MonoString* name, glm::ivec2* value);
			static void SetIVec3(Ref<MaterialInstance>* instance, MonoString* name, glm::ivec3* value);
			static void SetIVec4(Ref<MaterialInstance>* instance, MonoString* name, glm::ivec4* value);
		};

		struct MaterialAssetNative
		{
			static void				   Register();
			static Ref<MaterialAsset>* Constructor(Ref<ShaderAsset>* shaderAsset);
			static void				   Destructor(Ref<MaterialAsset>* instance);

			static void					  SetTexture(Ref<MaterialAsset>* instance, MonoString* name, Ref<Texture2D>* texture);
			static void					  SetTextureArr(Ref<MaterialAsset>* instance, MonoString* name, Ref<Texture2D>* texture, uint32_t index);

			static Ref<Material>*		  GetMaterial(Ref<MaterialAsset>* instance);
			static Ref<MaterialInstance>* GetMaterialInstance(Ref<MaterialAsset>* instance);
		};
	}
}