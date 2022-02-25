#pragma once

#include "XYZ/Asset/Animation/AnimationAsset.h"
#include "XYZ/Asset/Animation/AnimationController.h"
#include "XYZ/Asset/Animation/SkeletonAsset.h"

#include "XYZ/Script/ScriptWrappers.h"

namespace XYZ {
	namespace Script {
		struct AnimationAssetNative
		{
			static void Register();

			static Ref<AnimationAsset>* Constructor(MonoString* filepath, MonoString* name, Ref<SkeletonAsset>* skeletonInstance);
			static void Destructor(Ref<AnimationAsset>* instance);
		};

		struct AnimationControllerNative
		{
			static void Register();
			static Ref<AnimationController>* Constructor();
			static void Destructor(Ref<AnimationController>* instance);
		
			static void SetSkeletonAsset(Ref<AnimationController>* instance, Ref<SkeletonAsset>* skeletonInstance);
			static void SetState(Ref<AnimationController>* instance, MonoString* name);
			static void AddState(Ref<AnimationController>* instance, MonoString* name, Ref<AnimationAsset>* animInstance);
		};

		struct SkeletonAssetNative
		{
			static void Register();
			static Ref<SkeletonAsset>* Constructor(MonoString* path);
			static void Destructor(Ref<SkeletonAsset>* instance);
		};
	}
}