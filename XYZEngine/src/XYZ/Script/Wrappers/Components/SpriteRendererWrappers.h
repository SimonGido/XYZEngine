#pragma once
#include "XYZ/Script/ScriptWrappers.h"
#include "XYZ/Renderer/Material.h"
#include "XYZ/Renderer/SubTexture.h"
#include "XYZ/Asset/Renderer/MaterialAsset.h"

#include <glm/glm.hpp>

namespace XYZ {
	namespace Script {
		struct SpriteRendererNative
		{
			static void Register();
		private:
			static void		     SetColor(uint32_t entity, glm::vec4* inColor);
			static void		     SetSubTexture(uint32_t entity, Ref<SubTexture>* subTexture);
			static void		     SetMaterialAsset(uint32_t entity, Ref<MaterialAsset>* material);

			static Ref<SubTexture>*    GetSubTexture(uint32_t entity);
			static Ref<Material>*      GetMaterial(uint32_t entity);
			static Ref<MaterialAsset>* GetMaterialAsset(uint32_t entity);
			static void			       GetColor(uint32_t entity, glm::vec4* outColor);
		};
	}
}