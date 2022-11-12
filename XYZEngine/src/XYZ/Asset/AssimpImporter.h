#pragma once

#include <ozz/animation/runtime/animation.h>
#include <ozz/animation/runtime/sampling_job.h>
#include <ozz/animation/runtime/skeleton.h>
#include <ozz/base/containers/vector.h>
#include <ozz/base/maths/soa_transform.h>
#include <ozz/base/memory/unique_ptr.h>
#include <ozz/animation/offline/raw_skeleton.h>
#include <ozz/animation/offline/raw_animation.h>

#include <set>

struct aiNode;
struct aiAnimation;
struct aiNodeAnim;
struct aiScene;

namespace XYZ {
	class XYZ_API AssimpImporter
	{
	public:
		static bool ExtractRawSkeleton(const aiScene* scene, ozz::animation::offline::RawSkeleton& rawSkeleton);
		static bool ExtractRawAnimation(const aiAnimation* animation, const ozz::animation::Skeleton& skeleton, float samplingRate, ozz::animation::offline::RawAnimation& rawAnimation);

		static bool AreSameSkeleton(const ozz::animation::Skeleton& a, const ozz::animation::Skeleton& b);
		static std::vector<std::string> GetAnimationNames(const aiScene* scene);
		static aiAnimation* FindAnimation(const aiScene* scene, const std::string& name);
		
	private:
		static void traverseNode(aiNode* node, ozz::animation::offline::RawSkeleton& rawSkeleton, const std::set<std::string>& bones);
		static void traverseBone(aiNode* node, ozz::animation::offline::RawSkeleton::Joint& joint);
	};

}