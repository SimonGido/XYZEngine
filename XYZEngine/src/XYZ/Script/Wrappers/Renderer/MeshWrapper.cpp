#include "stdafx.h"
#include "MeshWrapper.h"

#include "XYZ/Scene/Scene.h"
#include "XYZ/Scene/SceneEntity.h"
#include "XYZ/Scene/Components.h"
#include "XYZ/Script/ScriptEngine.h"
#include "XYZ/Scene/Prefab.h"

#include <mono/jit/jit.h>
#include <mono/metadata/assembly.h>

#include <assimp/scene.h>

namespace XYZ {
	namespace Script {
		
		void MeshSourceNative::Register()
		{
			mono_add_internal_call("XYZ.MeshSource::Constructor_Native", Constructor);
			mono_add_internal_call("XYZ.MeshSource::Destructor_Native", Destructor);
		}
		Ref<MeshSource>* MeshSourceNative::Constructor(MonoString* filepath)
		{
			auto instance = Ref<MeshSource>::Create(mono_string_to_utf8(filepath));
			return new Ref<MeshSource>(instance);
		}
		void MeshSourceNative::Destructor(Ref<MeshSource>* instance)
		{
			delete instance;
		}
		void MeshNative::Register()
		{
			mono_add_internal_call("XYZ.Mesh::Constructor_Native", Constructor);
			mono_add_internal_call("XYZ.Mesh::Destructor_Native", Destructor);
		}
		Ref<Mesh>* MeshNative::Constructor(Ref<MeshSource>* meshSource)
		{
			auto instance = Ref<Mesh>::Create(*meshSource);
			return new Ref<Mesh>(instance);
		}
		void MeshNative::Destructor(Ref<Mesh>* instance)
		{
			delete instance;
		}
		void AnimatedMeshNative::Register()
		{
			mono_add_internal_call("XYZ.AnimatedMesh::Constructor_Native", Constructor);
			mono_add_internal_call("XYZ.AnimatedMesh::Destructor_Native", Destructor);
			mono_add_internal_call("XYZ.AnimatedMesh::CreateBones_Native", CreateBones);
		}
		Ref<AnimatedMesh>* AnimatedMeshNative::Constructor(Ref<MeshSource>* meshSource)
		{
			auto instance = Ref<AnimatedMesh>::Create(*meshSource);
			return new Ref<AnimatedMesh>(instance);
		}
		void AnimatedMeshNative::Destructor(Ref<AnimatedMesh>* instance)
		{
			delete instance;
		}
		MonoArray* AnimatedMeshNative::CreateBones(Ref<AnimatedMesh>* instance, uint32_t parent)
		{
			Ref<Scene> scene = ScriptEngine::GetCurrentSceneContext();
			XYZ_ASSERT(scene.Raw(), "No active scene!");

			SceneEntity ent(static_cast<entt::entity>(parent), scene.Raw());
			auto prefab = Ref<Prefab>::Create();
			prefab->Create(*instance, "Bones");
			SceneEntity root = prefab->Instantiate(scene, ent);
			
			auto& registry = scene->GetRegistry();
			auto tree = root.GetComponent<Relationship>().GetTree(registry);
			
			
			for (auto e : tree) // Go over the hierarchy
			{
				if (registry.any_of<AnimatedMeshComponent>(e)) // If has animated mesh component copy it's bone entities
				{
					auto& animatedMeshComponent = registry.get<AnimatedMeshComponent>(e);
					MonoArray* result = mono_array_new(mono_domain_get(), ScriptEngine::GetCoreClass("XYZ.Entity"), animatedMeshComponent.BoneEntities.size());
					uint32_t index = 0;
					for (auto boneEntity : animatedMeshComponent.BoneEntities)
					{		
						mono_array_set(result, entt::entity, index++, boneEntity);
					}
					return result;
				}
			}
			return nullptr;
		}
	}
}