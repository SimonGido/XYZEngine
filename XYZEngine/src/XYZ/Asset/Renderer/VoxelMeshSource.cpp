#include "stdafx.h"
#include "VoxelMeshSource.h"

#include <ogt_vox.h>

namespace XYZ {

	static glm::mat4 VoxMat4ToGLM(const ogt_vox_transform& voxTransform)
	{
		glm::mat4 result;
		memcpy(&result, &voxTransform, sizeof(glm::mat4));
		return result;
	}
	static uint32_t Index3D(int x, int y, int z, int width, int height)
	{
		return x + width * (y + height * z);
	}

	static void LoadSubmeshModel(VoxelSubmesh& submesh, const ogt_vox_model* voxModel)
	{
		submesh.ColorIndices.resize(voxModel->size_x * voxModel->size_y * voxModel->size_z);
		submesh.Width = voxModel->size_x;
		submesh.Height = voxModel->size_y;
		submesh.Depth = voxModel->size_z;

		for (uint32_t x = 0; x < voxModel->size_x; ++x)
		{
			for (uint32_t y = 0; y < voxModel->size_y; ++y)
			{
				for (uint32_t z = 0; z < voxModel->size_z; ++z)
				{
					const uint32_t index = Index3D(x, y, z, voxModel->size_x, voxModel->size_y);
					submesh.ColorIndices[index] = voxModel->voxel_data[index];
				}
			}
		}
	}

	static void LoadVoxelAnimation(VoxelAnimation& anim, const ogt_vox_instance& voxInstance)
	{
		anim.Loop = voxInstance.transform_anim.loop;
		anim.Transforms.resize(voxInstance.transform_anim.num_keyframes);
		for (uint32_t i = 0; i < voxInstance.transform_anim.num_keyframes; ++i)
		{
			const uint32_t frameIndex = voxInstance.transform_anim.keyframes[i].frame_index;
			anim.Transforms[frameIndex] = VoxMat4ToGLM(voxInstance.transform_anim.keyframes[i].transform);
		}
	}


	VoxelMeshSource::VoxelMeshSource(const std::string& filepath)
		:
		m_Filepath(filepath)
	{
		std::ifstream output(m_Filepath, std::ios::binary);
		std::vector<uint8_t> data(std::istreambuf_iterator<char>(output), {});
		auto scene = ogt_vox_read_scene(data.data(), data.size());

		memcpy(m_ColorPallete.data(), scene->palette.color, m_ColorPallete.size() * sizeof(VoxelColor));
		for (uint32_t i = 0; i < scene->num_models; ++i)
		{
			VoxelSubmesh& submesh = m_Submeshes.emplace_back();
			LoadSubmeshModel(submesh, scene->models[i]);
		}
		for (uint32_t i = 0; i < scene->num_instances; ++i)
		{
			VoxelInstance& instance = m_Instances.emplace_back();
			instance.Transform = VoxMat4ToGLM(scene->instances[i].transform);
			instance.GroupIndex = scene->instances[i].group_index;
			instance.SubmeshIndex = scene->instances[i].model_index;
			LoadVoxelAnimation(instance.Animation, scene->instances[i]);
		}
		for (uint32_t i = 0; i < scene->num_groups; ++i)
		{
			VoxelGroup& group = m_Groups.emplace_back();
			group.Transform = VoxMat4ToGLM(scene->groups[i].transform);
			group.ParentIndex = scene->groups[i].parent_group_index;
		}
	}
}