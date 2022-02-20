#include "AnimationController.h"

#include <ozz/animation/offline/raw_skeleton.h>
#include <ozz/animation/offline/skeleton_builder.h>
#include <ozz/animation/offline/raw_animation.h>
#include <ozz/animation/offline/animation_builder.h>
#include <ozz/animation/runtime/animation.h>
#include <ozz/animation/runtime/local_to_model_job.h>
#include <ozz/animation/runtime/sampling_job.h>
#include <ozz/base/span.h>

#include <iostream>

namespace XYZ {
	void AnimationController::Update(Timestep ts)
	{
		m_AnimationTime += ts;
		float ratio = m_AnimationTime / Animation->GetAnimation().duration();
		if (ratio >= 1.0f)
		{
			m_AnimationTime = 0.0f;
			ratio = 0.0f;
		}

		std::cout << ratio << std::endl;
		ozz::animation::SamplingJob sampling_job;
		sampling_job.animation = &Animation->GetAnimation();
		sampling_job.context = &m_SamplingContext;
		sampling_job.ratio = ratio;
		sampling_job.output = ozz::make_span(m_LocalSpaceSoaTransforms);
		if (!sampling_job.Run())
		{
			XYZ_ERROR("ozz animation sampling job failed!");
		}


		ozz::vector<ozz::math::Float4x4> boneTransforms(m_SkeletonAsset->GetSkeleton().num_joints());   // Note (0x):  performance? can we avoid constructing this every frame, every mesh?
		
		ozz::animation::LocalToModelJob ltm_job;
		ltm_job.skeleton = &m_SkeletonAsset->GetSkeleton();
		ltm_job.input = ozz::make_span(m_LocalSpaceSoaTransforms);
		ltm_job.output = ozz::make_span(boneTransforms);
		if (!ltm_job.Run())
		{
			XYZ_ERROR("ozz animation convertion to model space failed!");
		}
		
		m_Transforms.resize(boneTransforms.size());
		for (size_t i = 0; i < m_Transforms.size(); ++i)
		{
			memcpy(&m_Transforms[i], &boneTransforms[i], sizeof(ozz::math::Float4x4));
		}
	}
	void AnimationController::SetSkeletonAsset(const Ref<SkeletonAsset>& skeletonAsset)
	{
		m_SkeletonAsset = skeletonAsset;
		if (m_SkeletonAsset.Raw() && m_SkeletonAsset->IsValid())
		{
			m_SamplingContext.Resize(m_SkeletonAsset->GetSkeleton().num_joints());
			m_LocalSpaceSoaTransforms.resize(m_SkeletonAsset->GetSkeleton().num_soa_joints());
			m_LocalTranslations.resize(m_SkeletonAsset->GetSkeleton().num_joints());
			m_LocalScales.resize(m_SkeletonAsset->GetSkeleton().num_joints());
		}	
	}
	std::vector<glm::mat4> AnimationController::GetTransforms() const
	{
		return m_Transforms;
	}
}