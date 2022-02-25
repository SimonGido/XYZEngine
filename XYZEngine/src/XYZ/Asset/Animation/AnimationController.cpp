#include "AnimationController.h"

#include <ozz/animation/offline/raw_skeleton.h>
#include <ozz/animation/offline/skeleton_builder.h>
#include <ozz/animation/offline/raw_animation.h>
#include <ozz/animation/offline/animation_builder.h>
#include <ozz/animation/runtime/animation.h>
#include <ozz/animation/runtime/local_to_model_job.h>
#include <ozz/animation/runtime/sampling_job.h>
#include <ozz/base/span.h>

#define GLM_ENABLE_EXPERIMENTAL
#include <glm/gtx/quaternion.hpp>
#include <glm/gtc/type_ptr.hpp>

namespace XYZ {
	void AnimationController::Update(float& animationTime)
	{
		if (m_AnimationStates.empty())
			return;

		float ratio = animationTime / m_AnimationStates[m_StateIndex]->GetAnimation().duration();
		if (ratio >= 1.0f)
		{
			animationTime = 0.0f;
			ratio = 0.0f;
		}

		updateSampling(ratio);
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
			m_LocalRotations.resize(m_SkeletonAsset->GetSkeleton().num_joints());
			m_BoneTransforms.resize(m_SkeletonAsset->GetSkeleton().num_joints());
		}	
	}
	void AnimationController::SetCurrentState(const std::string& name)
	{
		for (size_t i = 0; i < m_AnimationNames.size(); ++i)
		{
			if (m_AnimationNames[i] == name)
			{
				m_StateIndex = i;
				return;
			}
		}
	}
	void AnimationController::AddState(const std::string_view name, const Ref<AnimationAsset>& animation)
	{
		m_AnimationNames.push_back(std::string(name));
		m_AnimationStates.push_back(animation);
	}
	const std::vector<ozz::math::Float4x4>& AnimationController::GetTransforms() const
	{
		return m_BoneTransforms;
	}
	void AnimationController::UpdateModel()
	{
		ozz::animation::LocalToModelJob ltm_job;
		ltm_job.skeleton = &m_SkeletonAsset->GetSkeleton();
		ltm_job.input = ozz::make_span(m_LocalSpaceSoaTransforms);
		ltm_job.output = ozz::make_span(m_BoneTransforms);
		if (!ltm_job.Run())
		{
			XYZ_ERROR("ozz animation convertion to model space failed!");
		}
	}
	void AnimationController::updateSampling(float ratio)
	{
		ozz::animation::SamplingJob sampling_job;
		sampling_job.animation = &m_AnimationStates[m_StateIndex]->GetAnimation();
		sampling_job.context = &m_SamplingContext;
		sampling_job.ratio = ratio;
		sampling_job.output = ozz::make_span(m_LocalSpaceSoaTransforms);
		if (!sampling_job.Run())
		{
			XYZ_ERROR("ozz animation sampling job failed!");
		}


		for (int i = 0; i < m_LocalSpaceSoaTransforms.size(); ++i)
		{
			ozz::math::SimdFloat4 translations[4];
			ozz::math::SimdFloat4 scales[4];
			ozz::math::SimdFloat4 rotations[4];

			ozz::math::Transpose3x4(&m_LocalSpaceSoaTransforms[i].translation.x, translations);
			ozz::math::Transpose3x4(&m_LocalSpaceSoaTransforms[i].scale.x, scales);
			ozz::math::Transpose4x4(&m_LocalSpaceSoaTransforms[i].rotation.x, rotations);

			for (int j = 0; j < 4; ++j)
			{
				auto index = i * 4 + j;
				if (index >= m_LocalTranslations.size())
					break;

				ozz::math::Store3PtrU(translations[j], glm::value_ptr(m_LocalTranslations[index]));
				ozz::math::Store3PtrU(scales[j], glm::value_ptr(m_LocalScales[index]));
				ozz::math::StorePtrU(rotations[j], glm::value_ptr(m_LocalRotations[index]));
			}
		}
	}
}