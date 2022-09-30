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

		void Update(float& animationTime);

		void SetSkeletonAsset(const Ref<SkeletonAsset>& skeletonAsset);
		void SetCurrentState(size_t index) { m_StateIndex = index; };
		void SetCurrentState(const std::string& name);
		void AddState(const std::string_view name, const Ref<AnimationAsset>& animation);

		size_t GetCurrentState() const { return m_StateIndex; }

		const glm::vec3& GetTranslation(size_t jointIndex) const { return m_LocalTranslations[jointIndex]; }
		const glm::vec3& GetScale(size_t jointIndex)	   const { return m_LocalScales[jointIndex]; }
		const glm::quat& GetRotation(size_t jointIndex)	   const { return m_LocalRotations[jointIndex]; }

		const Ref<SkeletonAsset>&				GetSkeleton()		 const { return m_SkeletonAsset; }
		const std::vector<std::string>&			GetStateNames()		 const { return m_AnimationNames; }
		const std::vector<Ref<AnimationAsset>>& GetAnimationStates() const { return m_AnimationStates; }
		const std::vector<ozz::math::Float4x4>& GetTransforms()		 const; 

		static AssetType GetStaticType() { return AssetType::AnimationController; }
		virtual AssetType GetAssetType() const override { return GetStaticType(); }


		void UpdateModel();
	private:
		void updateSampling(float ratio);
		

	private:
		Ref<SkeletonAsset> m_SkeletonAsset;
		ozz::animation::SamplingJob::Context m_SamplingContext;
		ozz::vector<ozz::math::SoaTransform> m_LocalSpaceSoaTransforms;

		std::vector<glm::vec3> m_LocalTranslations;
		std::vector<glm::vec3> m_LocalScales;
		std::vector<glm::quat> m_LocalRotations;

		std::vector<ozz::math::Float4x4> m_BoneTransforms;

		std::vector<Ref<AnimationAsset>> m_AnimationStates;
		std::vector<std::string>		 m_AnimationNames;
		size_t							 m_StateIndex = 0;

	};
}