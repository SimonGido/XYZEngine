#include "stdafx.h"
#include "MeshComponentWrapper.h"

#include "XYZ/Script/ScriptEngine.h"
#include "XYZ/Scene/SceneEntity.h"
#include "XYZ/Scene/Components.h"
#include "XYZ/Scene/Prefab.h"


#include <mono/jit/jit.h>
#include <mono/metadata/assembly.h>

namespace XYZ {
	namespace Script {
		void MeshComponentNative::Register()
		{
			mono_add_internal_call("XYZ.MeshComponent::GetMaterial_Native", GetMaterial);
			mono_add_internal_call("XYZ.MeshComponent::GetMesh_Native", GetMesh);

			mono_add_internal_call("XYZ.MeshComponent::SetMaterial_Native", SetMaterial);
			mono_add_internal_call("XYZ.MeshComponent::SetMesh_Native", SetMesh);
		}
		Ref<MaterialAsset>* MeshComponentNative::GetMaterial(uint32_t entity)
		{
			Ref<Scene> scene = ScriptEngine::GetCurrentSceneContext();
			XYZ_ASSERT(scene.Raw(), "No active scene!");

			SceneEntity ent(static_cast<entt::entity>(entity), scene.Raw());

			return new Ref<MaterialAsset>(ent.GetComponent<MeshComponent>().MaterialAsset);
		}
		Ref<Mesh>* MeshComponentNative::GetMesh(uint32_t entity)
		{
			Ref<Scene> scene = ScriptEngine::GetCurrentSceneContext();
			XYZ_ASSERT(scene.Raw(), "No active scene!");

			SceneEntity ent(static_cast<entt::entity>(entity), scene.Raw());

			return new Ref<Mesh>(ent.GetComponent<MeshComponent>().Mesh);
		}
		void MeshComponentNative::SetMaterial(uint32_t entity, Ref<MaterialAsset>* material)
		{
			Ref<Scene> scene = ScriptEngine::GetCurrentSceneContext();
			XYZ_ASSERT(scene.Raw(), "No active scene!");

			SceneEntity ent(static_cast<entt::entity>(entity), scene.Raw());
			ent.GetComponent<MeshComponent>().MaterialAsset = *material;
		}
		void MeshComponentNative::SetMesh(uint32_t entity, Ref<Mesh>* mesh)
		{
			Ref<Scene> scene = ScriptEngine::GetCurrentSceneContext();
			XYZ_ASSERT(scene.Raw(), "No active scene!");

			SceneEntity ent(static_cast<entt::entity>(entity), scene.Raw());
			ent.GetComponent<MeshComponent>().Mesh = *mesh;
		}
		void AnimatedMeshComponentNative::Register()
		{
			mono_add_internal_call("XYZ.AnimatedMeshComponent::GetMaterial_Native", GetMaterial);
			mono_add_internal_call("XYZ.AnimatedMeshComponent::GetMesh_Native", GetMesh);

			mono_add_internal_call("XYZ.AnimatedMeshComponent::SetMaterial_Native", SetMaterial);
			mono_add_internal_call("XYZ.AnimatedMeshComponent::SetMesh_Native", SetMesh);
		}
		Ref<MaterialAsset>* AnimatedMeshComponentNative::GetMaterial(uint32_t entity)
		{
			Ref<Scene> scene = ScriptEngine::GetCurrentSceneContext();
			XYZ_ASSERT(scene.Raw(), "No active scene!");

			SceneEntity ent(static_cast<entt::entity>(entity), scene.Raw());
			return new Ref<MaterialAsset>(ent.GetComponent<AnimatedMeshComponent>().MaterialAsset);
		}
		Ref<AnimatedMesh>* AnimatedMeshComponentNative::GetMesh(uint32_t entity)
		{
			Ref<Scene> scene = ScriptEngine::GetCurrentSceneContext();
			XYZ_ASSERT(scene.Raw(), "No active scene!");

			SceneEntity ent(static_cast<entt::entity>(entity), scene.Raw());
			return new Ref<AnimatedMesh>(ent.GetComponent<AnimatedMeshComponent>().Mesh);

		}
		MonoArray* AnimatedMeshComponentNative::GetBoneEntities(uint32_t entity)
		{
			Ref<Scene> scene = ScriptEngine::GetCurrentSceneContext();
			XYZ_ASSERT(scene.Raw(), "No active scene!");

			SceneEntity ent(static_cast<entt::entity>(entity), scene.Raw());
			auto& boneEntities = ent.GetComponent<AnimatedMeshComponent>().BoneEntities;

			MonoArray* result = mono_array_new(mono_domain_get(), ScriptEngine::GetCoreClass("XYZ.Entity"), boneEntities.size());

			uint32_t index = 0;
			for (auto boneEntity : boneEntities)
			{
				mono_array_set(result, entt::entity, index++, boneEntity);
			}
			return result;
		}
		void AnimatedMeshComponentNative::SetMaterial(uint32_t entity, Ref<MaterialAsset>* material)
		{
			Ref<Scene> scene = ScriptEngine::GetCurrentSceneContext();
			XYZ_ASSERT(scene.Raw(), "No active scene!");

			SceneEntity ent(static_cast<entt::entity>(entity), scene.Raw());
			ent.GetComponent<AnimatedMeshComponent>().MaterialAsset = *material;

		}
		void AnimatedMeshComponentNative::SetMesh(uint32_t entity, Ref<AnimatedMesh>* mesh)
		{
			Ref<Scene> scene = ScriptEngine::GetCurrentSceneContext();
			XYZ_ASSERT(scene.Raw(), "No active scene!");

			SceneEntity ent(static_cast<entt::entity>(entity), scene.Raw());
			ent.GetComponent<AnimatedMeshComponent>().Mesh = *mesh;
		}
	}
}