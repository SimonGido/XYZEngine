#include "stdafx.h"
#include "MeshWrapper.h"

#include <mono/jit/jit.h>
#include <mono/metadata/assembly.h>

namespace XYZ {
	namespace Script {
		void MeshSourceNative::Register()
		{
			mono_add_internal_call("XYZ.MeshSource::Constructor_Native", Constructor);
			mono_add_internal_call("XYZ.MeshSource::Destructor_Native", Destructor);
		}
		Ref<MeshSource>* MeshSourceNative::Constructor(const std::string& filepath)
		{
			auto instance = Ref<MeshSource>::Create(filepath);
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
	}
}