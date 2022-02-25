#include "stdafx.h"
#include "SkeletonAsset.h"
#include "XYZ/Asset/AssimpLog.h"

#include <assimp/scene.h>
#include <assimp/postprocess.h>
#include <assimp/Importer.hpp>

#include <glm/gtc/type_ptr.hpp>


#include <ozz/animation/offline/skeleton_builder.h>

#include <ozz/animation/offline/animation_builder.h>
#include <ozz/animation/runtime/animation.h>
#include <ozz/animation/runtime/local_to_model_job.h>
#include <ozz/animation/runtime/sampling_job.h>
#include <ozz/base/span.h>


#include "XYZ/Asset/AssimpImporter.h"

namespace XYZ {

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
		//		aiProcess_PopulateArmatureData |    // not currently using this data
		aiProcess_ValidateDataStructure;    // Validation 


	SkeletonAsset::SkeletonAsset(const std::string& filename)
		:
		m_FilePath(filename)
	{
		LogStream::Initialize();

		Assimp::Importer importer;
		const aiScene* scene = importer.ReadFile(filename, s_AnimationImportFlags);
	
		ozz::animation::offline::RawSkeleton rawSkeleton;
		if (AssimpImporter::ExtractRawSkeleton(scene, rawSkeleton))
		{
			ozz::animation::offline::SkeletonBuilder builder;
			m_Skeleton = builder(rawSkeleton);
			if (!m_Skeleton)
				XYZ_CORE_ERROR("Failed to build runtime skeleton from file {0}", filename);
		}
		else
		{
			XYZ_CORE_ERROR("No skeleton in file {0}", filename);
		}
	}

	SkeletonAsset::SkeletonAsset(const aiScene* scene, const std::string& filename)
		:
		m_FilePath(filename)
	{
		LogStream::Initialize();

		ozz::animation::offline::RawSkeleton rawSkeleton;
		if (AssimpImporter::ExtractRawSkeleton(scene, rawSkeleton))
		{
			ozz::animation::offline::SkeletonBuilder builder;
			m_Skeleton = builder(rawSkeleton);
			if (!m_Skeleton)
				XYZ_CORE_ERROR("Failed to build runtime skeleton from file {0}", filename);
		}
		else
		{
			XYZ_CORE_ERROR("No skeleton in file {0}", filename);
		}
	}

}