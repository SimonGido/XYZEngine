#include "stdafx.h"
#include "AssetImporter.h"
#include "AssimpImporter.h"

#include <assimp/scene.h>
#include <assimp/postprocess.h>
#include <assimp/Importer.hpp>

namespace XYZ {
	bool AssimpImporter::ExtractRawSkeleton(const aiScene* scene, ozz::animation::offline::RawSkeleton& rawSkeleton)
	{
		std::set<std::string> bones;
		if (!scene)
			return false;

		for (uint32_t meshIndex = 0; meshIndex < scene->mNumMeshes; ++meshIndex)
		{
			const aiMesh* mesh = scene->mMeshes[meshIndex];
			for (uint32_t boneIndex = 0; boneIndex < mesh->mNumBones; ++boneIndex)
			{
				bones.emplace(mesh->mBones[boneIndex]->mName.C_Str());
			}
		}
		if (bones.empty())
			return false;

		traverseNode(scene->mRootNode, rawSkeleton, bones);

		return true;
	}
	bool AssimpImporter::ExtractRawAnimation(const aiAnimation* animation, const ozz::animation::Skeleton& skeleton, float samplingRate, ozz::animation::offline::RawAnimation& rawAnimation)
	{
		std::unordered_map<std::string, size_t> jointIndices;
		for (size_t i = 0; i < skeleton.num_joints(); ++i)
		{
			jointIndices.emplace(skeleton.joint_names()[i], i);
		}
		samplingRate = samplingRate == 0.0f ? static_cast<float>(animation->mTicksPerSecond) : samplingRate;
		if (samplingRate < 0.0001)
		{
			samplingRate = 1.0;
		}
		
		rawAnimation.name = animation->mName.data;
		rawAnimation.duration = static_cast<float>(animation->mDuration) / samplingRate;

		std::set<std::tuple<size_t, aiNodeAnim*>> validChannels;
		for (uint32_t channelIndex = 0; channelIndex < animation->mNumChannels; ++channelIndex)
		{
			aiNodeAnim* nodeAnim = animation->mChannels[channelIndex];
			auto it = jointIndices.find(nodeAnim->mNodeName.C_Str());
			if (it != jointIndices.end())
			{
				size_t jointIndex = it->second;
				validChannels.emplace(jointIndex, nodeAnim);
			}
		}

		rawAnimation.tracks.resize(skeleton.num_joints());
		for (auto [jointIndex, nodeAnim] : validChannels)
		{
			for (uint32_t keyIndex = 0; keyIndex < nodeAnim->mNumPositionKeys; ++keyIndex)
			{
				aiVectorKey key = nodeAnim->mPositionKeys[keyIndex];
				float frameTime = std::clamp(static_cast<float>(key.mTime / samplingRate), 0.0f, rawAnimation.duration);
				rawAnimation.tracks[jointIndex].translations.push_back({ frameTime, ozz::math::Float3(static_cast<float>(key.mValue.x), static_cast<float>(key.mValue.y), static_cast<float>(key.mValue.z)) });
			}
			for (uint32_t keyIndex = 0; keyIndex < nodeAnim->mNumRotationKeys; ++keyIndex)
			{
				aiQuatKey key = nodeAnim->mRotationKeys[keyIndex];
				float frameTime = std::clamp(static_cast<float>(key.mTime / samplingRate), 0.0f, rawAnimation.duration);
				rawAnimation.tracks[jointIndex].rotations.push_back({ frameTime, ozz::math::Quaternion(static_cast<float>(key.mValue.x), static_cast<float>(key.mValue.y), static_cast<float>(key.mValue.z), static_cast<float>(key.mValue.w)) });
			}
			for (uint32_t keyIndex = 0; keyIndex < nodeAnim->mNumScalingKeys; ++keyIndex)
			{
				aiVectorKey key = nodeAnim->mScalingKeys[keyIndex];
				float frameTime = std::clamp(static_cast<float>(key.mTime / samplingRate), 0.0f, rawAnimation.duration);
				rawAnimation.tracks[jointIndex].scales.push_back({ frameTime, ozz::math::Float3(static_cast<float>(key.mValue.x), static_cast<float>(key.mValue.y), static_cast<float>(key.mValue.z)) });
			}
		}
		return true;
	}

	bool AssimpImporter::AreSameSkeleton(const ozz::animation::Skeleton& a, const ozz::animation::Skeleton& b)
	{
		ozz::span<const char* const> jointsA = a.joint_names();
		ozz::span<const char* const> jointsB = b.joint_names();

		bool areSame = false;
		if (jointsA.size() == jointsB.size())
		{
			areSame = true;
			for (size_t jointIndex = 0; jointIndex < jointsA.size(); ++jointIndex)
			{
				if (strcmp(jointsA[jointIndex], jointsB[jointIndex]) != 0)
				{
					areSame = false;
					break;
				}
			}
		}
		return areSame;
	}

	std::vector<std::string> AssimpImporter::GetAnimationNames(const aiScene* scene)
	{
		std::vector<std::string> animationNames;
		if (scene)
		{
			animationNames.reserve(scene->mNumAnimations);
			for (size_t i = 0; i < scene->mNumAnimations; ++i)
			{
				animationNames.emplace_back(scene->mAnimations[i]->mName.C_Str());
			}
		}
		return animationNames;
	}

	aiAnimation* AssimpImporter::FindAnimation(const aiScene* scene, const std::string& name)
	{
		for (size_t i = 0; i < scene->mNumAnimations; ++i)
		{
			if (std::string(scene->mAnimations[i]->mName.data) == name)
			{
				return scene->mAnimations[i];
			}
		}
		return nullptr;
	}

	void AssimpImporter::traverseNode(aiNode* node, ozz::animation::offline::RawSkeleton& rawSkeleton, const std::set<std::string>& bones)
	{
		bool isBone = (bones.find(node->mName.C_Str()) != bones.end());
		if (isBone)
		{
			rawSkeleton.roots.emplace_back();
			traverseBone(node, rawSkeleton.roots.back());
		}
		else
		{
			for (uint32_t nodeIndex = 0; nodeIndex < node->mNumChildren; ++nodeIndex)
			{
				traverseNode(node->mChildren[nodeIndex], rawSkeleton, bones);
			}
		}
	}
	void AssimpImporter::traverseBone(aiNode* node, ozz::animation::offline::RawSkeleton::Joint& joint)
	{
		aiMatrix4x4 mat = node->mTransformation;
		aiVector3D scale;
		aiQuaternion rotation;
		aiVector3D translation;
		mat.Decompose(scale, rotation, translation);

		joint.name = node->mName.C_Str();
		joint.transform.translation = ozz::math::Float3(translation.x, translation.y, translation.z);
		joint.transform.rotation = ozz::math::Quaternion(rotation.x, rotation.y, rotation.z, rotation.w);
		joint.transform.scale = ozz::math::Float3(scale.x, scale.y, scale.z);

		joint.children.resize(node->mNumChildren);
		for (uint32_t nodeIndex = 0; nodeIndex < node->mNumChildren; ++nodeIndex)
		{
			traverseBone(node->mChildren[nodeIndex], joint.children[nodeIndex]);
		}
	}
}