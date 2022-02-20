#include "stdafx.h"
#include "AssimpModel.h"


#include <assimp/scene.h>
#include <assimp/postprocess.h>
#include <assimp/Importer.hpp>

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

	AssimpModel::AssimpModel(const std::string& filepath)
		:
		m_FilePath(filepath)
	{
		Assimp::Importer importer;
		m_Scene = importer.ReadFile(m_FilePath, s_AnimationImportFlags);
	}
	const aiScene* AssimpModel::GetScene() const
	{
		return m_Scene;
	}
}