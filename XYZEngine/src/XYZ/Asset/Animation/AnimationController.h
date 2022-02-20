#pragma once
#include "XYZ/Core/Timestep.h"
#include "AnimationAsset.h"


#define GLM_ENABLE_EXPERIMENTAL
#include <glm/glm.hpp>
#include <glm/gtx/quaternion.hpp>

#include <ozz/animation/runtime/animation.h>
#include <ozz/animation/runtime/sampling_job.h>
#include <ozz/animation/runtime/skeleton.h>
#include <ozz/base/containers/vector.h>
#include <ozz/base/maths/soa_transform.h>
#include <ozz/base/memory/unique_ptr.h>

namespace XYZ {
	// Controls which animation (or animations) is playing on a mesh.
	class AnimationController : public Asset
	{
	public:
		virtual ~AnimationController() = default;

		void Update(Timestep ts);

		void SetSkeletonAsset(const Ref<SkeletonAsset>& skeletonAsset);

		Ref<AnimationAsset> Animation;
		
		std::vector<glm::mat4> GetTransforms() const;

	private:
		Ref<SkeletonAsset> m_SkeletonAsset;
		ozz::animation::SamplingJob::Context m_SamplingContext;
		ozz::vector<ozz::math::SoaTransform> m_LocalSpaceSoaTransforms;

		std::vector<glm::vec3> m_LocalTranslations;
		std::vector<glm::vec3> m_LocalScales;
		std::vector<glm::quat> m_LocalRotations;
		float m_AnimationTime = 0.0f;


		std::vector<glm::mat4> m_Transforms;
	};
}