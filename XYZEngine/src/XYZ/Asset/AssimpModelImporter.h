#pragma once

#include "Renderer/MeshSource.h"
#include "Animation/SkeletonAsset.h"
#include "Animation/AnimationAsset.h"

namespace XYZ {
	
	class XYZ_API AssimpModelImporter
	{
	public:
		AssimpModelImporter(const std::string& targetDirectory, const std::string& filepath);

		Ref<MeshSource> GetMeshSourceAsset() const { return m_MeshSourceAsset; }
		Ref<SkeletonAsset> GetSkeletonAsset() const { return m_SkeletonAsset; }
		const std::vector<Ref<AnimationAsset>>& GetAnimationAssets() const { return m_AnimationAssets; }

	private:
		Ref<MeshSource>					 m_MeshSourceAsset;
		Ref<SkeletonAsset>				 m_SkeletonAsset;
		std::vector<Ref<AnimationAsset>> m_AnimationAssets;
	};
}