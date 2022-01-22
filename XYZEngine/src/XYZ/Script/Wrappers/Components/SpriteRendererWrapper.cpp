#include "stdafx.h"
#include "SpriteRendererWrappers.h"

#include "XYZ/Script/ScriptEngine.h"
#include "XYZ/Scene/SceneEntity.h"

#include <mono/jit/jit.h>
#include <mono/metadata/assembly.h>

namespace XYZ {
	namespace Script {
		void SpriteRendererNative::Register()
		{
			mono_add_internal_call("XYZ.SpriteRenderer::SetColor_Native", SetColor);
			mono_add_internal_call("XYZ.SpriteRenderer::SetSubTexture_Native", SetSubTexture);
			mono_add_internal_call("XYZ.SpriteRenderer::SetMaterial_Native", SetMaterial);

			mono_add_internal_call("XYZ.SpriteRenderer::GetColor_Native", GetColor);
			mono_add_internal_call("XYZ.SpriteRenderer::GetSubTexture_Native", GetSubTexture);
			mono_add_internal_call("XYZ.SpriteRenderer::GetMaterial_Native", GetMaterial);
		}
		void SpriteRendererNative::SetColor(uint32_t entity, glm::vec4* inColor)
		{
			Ref<Scene> scene = ScriptEngine::GetCurrentSceneContext();
			XYZ_ASSERT(scene.Raw(), "No active scene!");

			SceneEntity ent(entity, scene.Raw());

			ent.GetComponent<SpriteRenderer>().Color = *inColor;
		}
		void SpriteRendererNative::SetSubTexture(uint32_t entity, Ref<SubTexture>* subTexture)
		{
			Ref<Scene> scene = ScriptEngine::GetCurrentSceneContext();
			XYZ_ASSERT(scene.Raw(), "No active scene!");

			SceneEntity ent(entity, scene.Raw());

			ent.GetComponent<SpriteRenderer>().SubTexture = *subTexture;
		}
		void SpriteRendererNative::SetMaterial(uint32_t entity, Ref<Material>* material)
		{
			Ref<Scene> scene = ScriptEngine::GetCurrentSceneContext();
			XYZ_ASSERT(scene.Raw(), "No active scene!");

			SceneEntity ent(entity, scene.Raw());

			ent.GetComponent<SpriteRenderer>().Material = *material;
		}
		Ref<SubTexture>* SpriteRendererNative::GetSubTexture(uint32_t entity)
		{
			Ref<Scene> scene = ScriptEngine::GetCurrentSceneContext();
			XYZ_ASSERT(scene.Raw(), "No active scene!");

			SceneEntity ent(entity, scene.Raw());
			return &ent.GetComponent<SpriteRenderer>().SubTexture;
		}
		Ref<Material>* SpriteRendererNative::GetMaterial(uint32_t entity)
		{
			Ref<Scene> scene = ScriptEngine::GetCurrentSceneContext();
			XYZ_ASSERT(scene.Raw(), "No active scene!");

			SceneEntity ent(entity, scene.Raw());
			return &ent.GetComponent<SpriteRenderer>().Material;
		}
		void SpriteRendererNative::GetColor(uint32_t entity, glm::vec4* outColor)
		{
			Ref<Scene> scene = ScriptEngine::GetCurrentSceneContext();
			XYZ_ASSERT(scene.Raw(), "No active scene!");

			SceneEntity ent(entity, scene.Raw());
			*outColor = ent.GetComponent<SpriteRenderer>().Color;
		}
	}
}