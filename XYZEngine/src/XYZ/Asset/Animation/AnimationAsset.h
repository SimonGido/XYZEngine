#pragma once

#include "XYZ/Asset/Asset.h"
#include "SkeletonAsset.h"

#define GLM_ENABLE_EXPERIMENTAL
#include <glm/glm.hpp>
#include <glm/gtx/quaternion.hpp>

#include <ozz/animation/runtime/animation.h>
#include <ozz/animation/runtime/sampling_job.h>
#include <ozz/animation/runtime/skeleton.h>
#include <ozz/base/containers/vector.h>
#include <ozz/base/maths/soa_transform.h>
#include <ozz/base/memory/unique_ptr.h>
#include <ozz/animation/offline/raw_skeleton.h>


struct aiScene;

namespace XYZ {

	class AnimationAsset : public Asset
	{
	public:
		AnimationAsset(const std::string& filename, const std::string& animationName, Ref<SkeletonAsset> skeleton);
		virtual ~AnimationAsset() = default;

		const std::string& GetFilePath() const { return m_FilePath; }

		static AssetType GetStaticType() { return AssetType::Animation; }
		virtual AssetType GetAssetType() const override { return GetStaticType(); }


		const ozz::animation::Animation& GetAnimation() const { XYZ_ASSERT(m_Animation, "Attempted to access null animation!"); return *m_Animation; }

	private:
		AnimationAsset(const aiScene* scene, const std::string& filename, const std::string& animationName, Ref<SkeletonAsset> skeleton);

		void load(const aiScene* scene, const std::string& animationName, Ref<SkeletonAsset> skeleton);

	private:
		ozz::unique_ptr<ozz::animation::Animation> m_Animation;
		std::string m_FilePath;
		std::string m_AnimationName;

		friend class AssimpModelSerializer;
	};
}