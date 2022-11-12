#include "stdafx.h"
#include "SpriteRendererWrappers.h"

#include "XYZ/Script/ScriptEngine.h"
#include "XYZ/Scene/SceneEntity.h"
#include "XYZ/Scene/Components.h"
#include "XYZ/Scene/Prefab.h"


#include <mono/jit/jit.h>
#include <mono/metadata/assembly.h>

namespace XYZ {
	namespace Script {
		void SpriteRendererNative::Register()
		{
			mono_add_internal_call("XYZ.SpriteRenderer::SetColor_Native", SetColor);
			mono_add_internal_call("XYZ.SpriteRenderer::SetSubTexture_Native", SetSubTexture);
			mono_add_internal_call("XYZ.SpriteRenderer::SetMaterialAsset_Native", SetMaterialAsset);

			mono_add_internal_call("XYZ.SpriteRenderer::GetColor_Native", GetColor);
			mono_add_internal_call("XYZ.SpriteRenderer::GetSubTexture_Native", GetSubTexture);
			mono_add_internal_call("XYZ.SpriteRenderer::GetMaterial_Native", GetMaterial);
			mono_add_internal_call("XYZ.SpriteRenderer::GetMaterialAsset_Native", GetMaterialAsset);
		}
		void SpriteRendererNative::SetColor(uint32_t entity, glm::vec4* inColor)
		{
			Ref<Scene> scene = ScriptEngine::GetCurrentSceneContext();
			XYZ_ASSERT(scene.Raw(), "No active scene!");

			SceneEntity ent(static_cast<entt::entity>(entity), scene.Raw());

			ent.GetComponent<SpriteRenderer>().Color = *inColor;
		}
		void SpriteRendererNative::SetSubTexture(uint32_t entity, Ref<SubTexture>* subTexture)
		{
			Ref<Scene> scene = ScriptEngine::GetCurrentSceneContext();
			XYZ_ASSERT(scene.Raw(), "No active scene!");

			SceneEntity ent(static_cast<entt::entity>(entity), scene.Raw());

			ent.GetComponent<SpriteRenderer>().SubTexture = *subTexture;
		}
		void SpriteRendererNative::SetMaterialAsset(uint32_t entity, Ref<MaterialAsset>* material)
		{
			Ref<Scene> scene = ScriptEngine::GetCurrentSceneContext();
			XYZ_ASSERT(scene.Raw(), "No active scene!");
			
			SceneEntity ent(static_cast<entt::entity>(entity), scene.Raw());
			
			ent.GetComponent<SpriteRenderer>().Material = *material;
		}
		Ref<SubTexture>* SpriteRendererNative::GetSubTexture(uint32_t entity)
		{
			Ref<Scene> scene = ScriptEngine::GetCurrentSceneContext();
			XYZ_ASSERT(scene.Raw(), "No active scene!");

			SceneEntity ent(static_cast<entt::entity>(entity), scene.Raw());
			return new Ref<SubTexture>(ent.GetComponent<SpriteRenderer>().SubTexture);
		}
		Ref<Material>* SpriteRendererNative::GetMaterial(uint32_t entity)
		{
			Ref<Scene> scene = ScriptEngine::GetCurrentSceneContext();
			XYZ_ASSERT(scene.Raw(), "No active scene!");

			SceneEntity ent(static_cast<entt::entity>(entity), scene.Raw());
			return new Ref<Material>(ent.GetComponent<SpriteRenderer>().Material->GetMaterial());
		}
		Ref<MaterialAsset>* SpriteRendererNative::GetMaterialAsset(uint32_t entity)
		{
			Ref<Scene> scene = ScriptEngine::GetCurrentSceneContext();
			XYZ_ASSERT(scene.Raw(), "No active scene!");

			SceneEntity ent(static_cast<entt::entity>(entity), scene.Raw());
			return new Ref<MaterialAsset>(ent.GetComponent<SpriteRenderer>().Material);
		}
		void SpriteRendererNative::GetColor(uint32_t entity, glm::vec4* outColor)
		{
			Ref<Scene> scene = ScriptEngine::GetCurrentSceneContext();
			XYZ_ASSERT(scene.Raw(), "No active scene!");

			SceneEntity ent(static_cast<entt::entity>(entity), scene.Raw());
			*outColor = ent.GetComponent<SpriteRenderer>().Color;
		}
	}
}