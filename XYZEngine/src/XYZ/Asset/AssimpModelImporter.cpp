#include "stdafx.h"
#include "AssimpModelImporter.h"

#include "AssetManager.h"


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
		//		aiProcess_PopulateArmatureData |    // not currently using this data
		aiProcess_ValidateDataStructure;    // Validation 

namespace XYZ {
	AssimpModelImporter::AssimpModelImporter(const std::string& targetDirectory, const std::string& filepath)
	{
		Assimp::Importer importer;
		const aiScene* scene = importer.ReadFile(filepath, s_AnimationImportFlags);
		if (!scene)
			return;

		std::string name = Utils::GetFilenameWithoutExtension(filepath);
		
		const std::filesystem::path meshPath = targetDirectory + "/" + name + "." + Asset::GetExtension(AssetType::MeshSource);
		const std::filesystem::path skeletonPath = targetDirectory + "/" + "." + Asset::GetExtension(AssetType::Skeleton);

		if (AssetManager::Exist(meshPath))
		{
			m_MeshSourceAsset = AssetManager::GetAsset<MeshSource>(meshPath);
		}
		else 
		{
			m_MeshSourceAsset = AssetManager::CreateAsset<MeshSource>(name + "." + Asset::GetExtension(AssetType::MeshSource), targetDirectory, scene, filepath);
		}
		if (AssetManager::Exist(skeletonPath))
		{
			m_SkeletonAsset = AssetManager::GetAsset<SkeletonAsset>(skeletonPath);
		}
		else
		{
			m_SkeletonAsset = AssetManager::CreateAsset<SkeletonAsset>(name + "." + Asset::GetExtension(AssetType::Skeleton), targetDirectory, scene, filepath);
		}

		for (uint32_t i = 0; i < scene->mNumAnimations; ++i)
		{
			const std::filesystem::path animPath = targetDirectory + "/" + name + "." + Asset::GetExtension(AssetType::Animation);
			if (AssetManager::Exist(animPath))
			{
				m_AnimationAssets.push_back(AssetManager::GetAsset<AnimationAsset>(animPath));
			}
			else
			{
				m_AnimationAssets.push_back(AssetManager::CreateAsset<AnimationAsset>(name + "." + Asset::GetExtension(AssetType::Animation), targetDirectory,
					scene, filepath, std::string(scene->mAnimations[i]->mName.data), m_SkeletonAsset)
				);
			}
		}
	}
}