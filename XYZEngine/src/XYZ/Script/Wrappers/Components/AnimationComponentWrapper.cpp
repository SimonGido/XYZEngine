#include "stdafx.h"
#include "AnimationComponentWrapper.h"

#include <mono/jit/jit.h>
#include <mono/metadata/assembly.h>

namespace XYZ {
	namespace Script {
		void AnimationComponentNative::Register()
		{
		}
		Ref<AnimationController>* AnimationComponentNative::GetController(uint32_t entity)
		{
			return nullptr;
		}
		void AnimationComponentNative::GetBoneEntities(uint32_t entity, MonoArray* outputBoneEntities)
		{
		}
		float AnimationComponentNative::GetAnimationTime(uint32_t entity)
		{
			return 0.0f;
		}
		bool AnimationComponentNative::GetPlaying(uint32_t entity)
		{
			return false;
		}
	}
}