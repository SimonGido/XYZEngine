#pragma once
#include "XYZ/Script/ScriptWrappers.h"

#include "XYZ/Scene/Components.h"

namespace XYZ {
	namespace Script {

		struct AnimationComponentNative
		{
			static void Register();
		private:
			static Ref<AnimationController>* GetController(uint32_t entity);
			static void                      GetBoneEntities(uint32_t entity, MonoArray* outputBoneEntities);
			static float                     GetAnimationTime(uint32_t entity);
			static bool                      GetPlaying(uint32_t entity);
		};

	}
}