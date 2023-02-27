#pragma once

#include "XYZ/Asset/Renderer/MaterialAsset.h"
#include "XYZ/Renderer/Mesh.h"

#include "XYZ/Script/ScriptWrappers.h"

namespace XYZ {
	namespace Script {

		struct MeshComponentNative
		{
			static void Register();
		private:
			static Ref<MaterialAsset>* GetMaterial(uint32_t entity);
			static Ref<StaticMesh>* GetMesh(uint32_t entity);

			static void SetMaterial(uint32_t entity, Ref<MaterialAsset>* material);
			static void SetMesh(uint32_t entity, Ref<StaticMesh>* mesh);
		};


		struct AnimatedMeshComponentNative
		{
			static void Register();
		private:
			static Ref<MaterialAsset>* GetMaterial(uint32_t entity);
			static Ref<AnimatedMesh>*  GetMesh(uint32_t entity);
			static MonoArray*		   GetBoneEntities(uint32_t entity);


			static void SetMaterial(uint32_t entity, Ref<MaterialAsset>* material);
			static void SetMesh(uint32_t entity, Ref<AnimatedMesh>* mesh);
		};
	}
}