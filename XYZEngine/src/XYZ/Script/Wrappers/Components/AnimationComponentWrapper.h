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
			static  MonoArray*				 GetBoneEntities(uint32_t entity);
			static float                     GetAnimationTime(uint32_t entity);
			static bool                      GetPlaying(uint32_t entity);

			static void						 SetController(uint32_t entity, Ref<AnimationController>* controllerInstance);
			static void						 SetPlaying(uint32_t entity, bool play);
			static void						 SetAnimationTime(uint32_t entity, float time);
			static void						 SetBoneEntities(uint32_t entity, MonoArray* boneEntities);
		};

	}
}