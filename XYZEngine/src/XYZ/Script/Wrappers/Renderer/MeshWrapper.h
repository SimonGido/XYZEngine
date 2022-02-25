#pragma once
#include "XYZ/Renderer/Mesh.h"

#include "XYZ/Script/ScriptWrappers.h"

namespace XYZ {
	namespace Script {
		struct MeshSourceNative
		{
			static void Register();
			static Ref<MeshSource>* Constructor(MonoString* filepath);
			static void Destructor(Ref<MeshSource>* instance);
		};

		struct MeshNative
		{
			static void Register();
			static Ref<Mesh>* Constructor(Ref<MeshSource>* meshSource);
			static void Destructor(Ref<Mesh>* instance);
		};

		struct AnimatedMeshNative
		{
			static void Register();
			static Ref<AnimatedMesh>* Constructor(Ref<MeshSource>* meshSource);
			static void Destructor(Ref<AnimatedMesh>* instance);
		
			static void CreateBones(Ref<AnimatedMesh>* instance, uint32_t parent);
		};
	}
}