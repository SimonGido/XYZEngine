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

	struct XYZ_API SamplingContext
	{
		SamplingContext();
		SamplingContext(const SamplingContext& other);

		SamplingContext& operator=(const SamplingContext& other);

		std::vector<glm::vec3> LocalTranslations;
		std::vector<glm::vec3> LocalScales;
		std::vector<glm::quat> LocalRotations;
	
	private:
		void resize(uint32_t size);
		void resizeSao(uint32_t size);


	private:
		ozz::animation::SamplingJob::Context m_Context;
		ozz::vector<ozz::math::SoaTransform> m_LocalSpaceSoaTransforms;
	
		uint32_t m_SaoSize = 0;
		uint32_t m_Size = 0;

		friend class AnimationController;
	};

	// Controls which animation (or animations) is playing on a mesh.
	class XYZ_API AnimationController : public Asset
	{
	public:
		virtual ~AnimationController() = default;

		void Update(float& animationTime, SamplingContext& context);

		void SetSkeletonAsset(const Ref<SkeletonAsset>& skeletonAsset);
		void SetCurrentState(size_t index) { m_StateIndex = index; };
		void SetCurrentState(const std::string& name);
		void AddState(const std::string_view name, const Ref<AnimationAsset>& animation);
		void SetState(size_t index, const std::string_view name, const Ref<AnimationAsset>& animation);

		size_t GetCurrentState() const { return m_StateIndex; }

		const Ref<SkeletonAsset>&				GetSkeleton()		 const { return m_SkeletonAsset; }
		const std::vector<std::string>&			GetStateNames()		 const { return m_AnimationNames; }
		const std::vector<Ref<AnimationAsset>>& GetAnimationStates() const { return m_AnimationStates; }
		

		static AssetType GetStaticType() { return AssetType::AnimationController; }
		virtual AssetType GetAssetType() const override { return GetStaticType(); }

	private:
		void updateSampling(float ratio, SamplingContext& context);
		

	private:
		Ref<SkeletonAsset>				 m_SkeletonAsset;
		std::vector<Ref<AnimationAsset>> m_AnimationStates;
		std::vector<std::string>		 m_AnimationNames;

		
		size_t m_StateIndex = 0;
	};
}