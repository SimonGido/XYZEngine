#include "stdafx.h"
#include "AnimationAsset.h"

#include "XYZ/Asset/AssimpLog.h"
#include "XYZ/Asset/AssimpImporter.h"


#include <ozz/animation/offline/skeleton_builder.h>

#include <ozz/animation/offline/animation_builder.h>
#include <ozz/animation/runtime/animation.h>
#include <ozz/animation/runtime/local_to_model_job.h>
#include <ozz/animation/runtime/sampling_job.h>
#include <ozz/base/span.h>

#include <assimp/scene.h>
#include <assimp/postprocess.h>
#include <assimp/Importer.hpp>

static const uint32_t s_AnimationImportFlags =
					aiProcess_CalcTangentSpace |        // Create binormals/tangents just in case
					aiProcess_Triangulate |             // Make sure we're triangles
					aiProcess_SortByPType |             // Split meshes by primitive type
					aiProcess_GenNormals |              // Make sure we have legit normals
					aiProcess_GenUVCoords |             // Convert UVs if required
					//		aiProcess_OptimizeGraph |
					aiProcess_OptimizeMeshes |          // Batch draws where possible
					aiProcess_JoinIdenticalVertices |
					aiProcess_LimitBoneWeights |        // If more than N (=4) bone weights, discard least influencing bones and renormalise sum to 1
					aiProcess_GlobalScale |             // e.g. convert cm to m for fbx import (and other formats where cm is native)
					aiProcess_PopulateArmatureData |    // not currently using this data
					aiProcess_ValidateDataStructure;    // Validation 

namespace XYZ {
	AnimationAsset::AnimationAsset(const std::string& filename, const std::string& animationName, Ref<SkeletonAsset> skeleton)
		:
		m_FilePath(filename),
		m_AnimationName(animationName)
	{
		LogStream::Initialize();

		XYZ_INFO("Loading animation: {0}", m_FilePath);

		if (!skeleton.Raw() || !skeleton->IsValid())
		{
			XYZ_ERROR("Invalid skeleton passed to animation asset for file '{0}'", m_FilePath);
			SetFlag(AssetFlag::Invalid);
			return;
		}
		Assimp::Importer importer;
		const aiScene* scene = importer.ReadFile(m_FilePath, s_AnimationImportFlags);
		if (!scene || !scene->HasAnimations())
		{
			XYZ_ERROR("Failed to load animation from file '{0}'", m_FilePath);
			SetFlag(AssetFlag::Invalid);
			return;
		}
		load(scene, animationName, skeleton);	
	}
	AnimationAsset::AnimationAsset(const aiScene* scene, const std::string& filename, const std::string& animationName, Ref<SkeletonAsset> skeleton)
		:
		m_FilePath(filename),
		m_AnimationName(animationName)
	{
		if (!scene || !scene->HasAnimations())
		{
			XYZ_ERROR("Failed to load animation from file '{0}'", m_FilePath);
			SetFlag(AssetFlag::Invalid);
			return;
		}
		load(scene, animationName, skeleton);
	}
	void AnimationAsset::load(const aiScene* scene, const std::string& animationName, Ref<SkeletonAsset> skeleton)
	{
		auto animationNames = AssimpImporter::GetAnimationNames(scene);
		if (animationNames.empty())
		{
			XYZ_ERROR("Failed to load animation from file: {0}", m_FilePath);
			SetFlag(AssetFlag::Invalid);
			return;
		}
		aiAnimation* animation = AssimpImporter::FindAnimation(scene, m_AnimationName);
		if (animation == nullptr)
		{
			XYZ_ERROR("Failed to load animation from file: {0}", m_FilePath);
			SetFlag(AssetFlag::Invalid);
			return;
		}
		ozz::animation::offline::RawAnimation rawAnimation;
		if (AssimpImporter::ExtractRawAnimation(animation, skeleton->GetSkeleton(), 0.0f, rawAnimation))
		{
			if (rawAnimation.Validate())
			{
				ozz::animation::offline::AnimationBuilder builder;
				m_Animation = builder(rawAnimation);
				if (!m_Animation)
				{
					XYZ_ERROR("Failed to build runtime animation for '{}' from file '{}'", animationNames.front(), m_FilePath);
					SetFlag(AssetFlag::Invalid);
					return;
				}
			}
			else
			{
				XYZ_ERROR("Failed validation for animation {} from file {}", m_AnimationName, m_FilePath);
				SetFlag(AssetFlag::Invalid);
				return;
			}
		}
	}
}
