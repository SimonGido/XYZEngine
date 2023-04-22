#pragma once
#include "XYZ/Asset/Asset.h"

#include <glm/glm.hpp>
#include <glm/ext/matrix_transform.hpp>
#include <glm/gtc/quaternion.hpp>

#define GLM_ENABLE_EXPERIMENTAL
#include <glm/gtx/quaternion.hpp>
#include <glm/gtx/matrix_decompose.hpp>

namespace XYZ {

	struct VoxelSubmesh
	{		
		uint32_t Width, Height, Depth;
		std::vector<uint8_t> ColorIndices;
	};

	struct VoxelAnimation
	{
		std::vector<glm::mat4> Transforms;
		std::vector<uint32_t> Keyframes;
		bool Loop;
	};

	struct VoxelInstance
	{
		glm::mat4 Transform;
		uint32_t  GroupIndex;
		uint32_t  SubmeshIndex;
		VoxelAnimation Animation;
	};

	struct VoxelGroup
	{
		glm::mat4 Transform;
		uint32_t  ParentIndex;
	};

	struct VoxelColor
	{
		uint8_t R, G, B, A;
	};

	class VoxelMeshSource : public Asset
	{
	public:
		VoxelMeshSource(const std::string& filepath);

		virtual AssetType GetAssetType() const override { return AssetType::VoxelMeshSource; }
		static AssetType	GetStaticType() { return AssetType::VoxelMeshSource; }

		const std::array<VoxelColor, 256>&	GetColorPallete() const { return m_ColorPallete; }
		const std::vector<VoxelSubmesh>&	GetSubmeshes()	  const { return m_Submeshes; }
		const std::vector<VoxelInstance>&	GetInstances()	  const { return m_Instances; }
		const std::vector<VoxelGroup>&		GetGroups()		  const { return m_Groups; }

	private:
		std::string m_Filepath;
		std::array<VoxelColor, 256> m_ColorPallete;
		std::vector<VoxelSubmesh>  m_Submeshes;
		std::vector<VoxelInstance> m_Instances;
		std::vector<VoxelGroup>    m_Groups;
	};
}