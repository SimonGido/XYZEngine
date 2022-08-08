#include "stdafx.h"
#include "AnimationWrapper.h"

#include <mono/jit/jit.h>
#include <mono/metadata/assembly.h>

namespace XYZ {
	namespace Script {
		void AnimationAssetNative::Register()
		{
			mono_add_internal_call("XYZ.AnimationAsset::Constructor_Native", Constructor);
			mono_add_internal_call("XYZ.AnimationAsset::Destructor_Native", Destructor);
		}
		Ref<AnimationAsset>* AnimationAssetNative::Constructor(MonoString* filepath, MonoString* name, Ref<SkeletonAsset>* skeletonInstance)
		{
			Ref<AnimationAsset> animation = Ref<AnimationAsset>::Create(mono_string_to_utf8(filepath), mono_string_to_utf8(name), *skeletonInstance);
			return new Ref<AnimationAsset>(animation);
		}
		void AnimationAssetNative::Destructor(Ref<AnimationAsset>* instance)
		{
			delete instance;
		}
		void AnimationControllerNative::Register()
		{
			mono_add_internal_call("XYZ.AnimationController::Constructor_Native", Constructor);
			mono_add_internal_call("XYZ.AnimationController::Destructor_Native", Destructor);
			mono_add_internal_call("XYZ.AnimationController::SetSkeletonAsset_Native", SetSkeletonAsset);
			mono_add_internal_call("XYZ.AnimationController::SetState_Native", SetState);
			mono_add_internal_call("XYZ.AnimationController::AddState_Native", AddState);
		}
		Ref<AnimationController>* AnimationControllerNative::Constructor()
		{
			Ref<AnimationController> controller = Ref<AnimationController>::Create();
			return new Ref<AnimationController>(controller);
		}
		void AnimationControllerNative::Destructor(Ref<AnimationController>* instance)
		{
			delete instance;
		}
		void AnimationControllerNative::SetSkeletonAsset(Ref<AnimationController>* instance, Ref<SkeletonAsset>* skeletonInstance)
		{
			(*instance)->SetSkeletonAsset(*skeletonInstance);
		}
		void AnimationControllerNative::SetState(Ref<AnimationController>* instance, MonoString* name)
		{
			std::string strName(mono_string_to_utf8(name));
			(*instance)->SetCurrentState(strName);
		}
		void AnimationControllerNative::AddState(Ref<AnimationController>* instance, MonoString* name, Ref<AnimationAsset>* animInstance)
		{
			(*instance)->AddState(mono_string_to_utf8(name), *animInstance);
		}
		void SkeletonAssetNative::Register()
		{
			mono_add_internal_call("XYZ.SkeletonAsset::Constructor_Native", Constructor);
			mono_add_internal_call("XYZ.SkeletonAsset::Destructor_Native", Destructor);
		}
		Ref<SkeletonAsset>* SkeletonAssetNative::Constructor(MonoString* path)
		{
			Ref<SkeletonAsset> skeleton = Ref<SkeletonAsset>::Create(mono_string_to_utf8(path));
			return new Ref<SkeletonAsset>(skeleton);
		}
		void SkeletonAssetNative::Destructor(Ref<SkeletonAsset>* instance)
		{
			delete instance;
		}
	}
}