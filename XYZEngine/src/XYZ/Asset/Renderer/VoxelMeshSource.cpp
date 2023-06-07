#include "stdafx.h"
#include "VoxelMeshSource.h"

#include <ogt_vox.h>

namespace XYZ {


	static AABB VoxelModelToAABB(const glm::mat4& transform, uint32_t width, uint32_t height, uint32_t depth, float voxelSize)
	{
		AABB result;
		result.Min = glm::vec3(0.0f);
		result.Max = glm::vec3(width, height, depth) * voxelSize;

		result = result.TransformAABB(transform);
		return result;
	}
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

	static void LoadVoxelAnimation(VoxelTransformAnimation& anim, const ogt_vox_instance& voxInstance)
	{
		anim.Loop = voxInstance.transform_anim.loop;
		anim.Transforms.resize(voxInstance.transform_anim.num_keyframes);
		for (uint32_t i = 0; i < voxInstance.transform_anim.num_keyframes; ++i)
		{
			const uint32_t frameIndex = voxInstance.transform_anim.keyframes[i].frame_index;
			anim.Transforms[frameIndex] = VoxMat4ToGLM(voxInstance.transform_anim.keyframes[i].transform);
		}
	}

	static void LoadVoxelModelAnimation(VoxelModelAnimation& anim, const ogt_vox_instance& voxInstance)
	{
		anim.Loop = voxInstance.model_anim.loop;
		anim.SubmeshIndices.resize(voxInstance.model_anim.num_keyframes);
		for (uint32_t i = 0; i < voxInstance.model_anim.num_keyframes; ++i)
		{
			const uint32_t frameIndex = voxInstance.model_anim.keyframes[i].frame_index;
			anim.SubmeshIndices[frameIndex] = voxInstance.model_anim.keyframes[i].model_index;
		}
	}


	VoxelMeshSource::VoxelMeshSource(const std::string& filepath)
		:
		m_Filepath(filepath),
		m_NumVoxels(0)
	{
		std::ifstream output(m_Filepath, std::ios::binary);
		std::vector<uint8_t> data(std::istreambuf_iterator<char>(output), {});
		auto scene = ogt_vox_read_scene_with_flags(data.data(), data.size(), k_read_scene_flags_keyframes);

		memcpy(m_ColorPallete.data(), scene->palette.color, m_ColorPallete.size() * sizeof(VoxelColor));
		for (uint32_t i = 0; i < scene->num_models; ++i)
		{
			VoxelSubmesh& submesh = m_Submeshes.emplace_back();
			LoadSubmeshModel(submesh, scene->models[i]);
			m_NumVoxels += static_cast<uint32_t>(submesh.ColorIndices.size());
		}

		for (uint32_t i = 0; i < scene->num_instances; ++i)
		{
			VoxelInstance& instance = m_Instances.emplace_back();
			instance.SubmeshIndex = scene->instances[i].model_index;
			const auto& submesh = m_Submeshes[instance.SubmeshIndex];
			
			glm::vec3 centerTranslation = -glm::vec3(
				static_cast<float>(submesh.Width) / 2.0f * submesh.VoxelSize,
				static_cast<float>(submesh.Height) / 2.0f * submesh.VoxelSize,
				static_cast<float>(submesh.Depth) / 2.0f * submesh.VoxelSize
			);
			centerTranslation = glm::vec3(0.0f);
			instance.Transform = VoxMat4ToGLM(scene->instances[i].transform) * glm::translate(glm::mat4(1.0f), centerTranslation);
			AABB::Union(m_AABB, VoxelModelToAABB(instance.Transform, submesh.Width, submesh.Height, submesh.Depth, submesh.VoxelSize));

			LoadVoxelAnimation(instance.TransformAnimation, scene->instances[i]);
			LoadVoxelModelAnimation(instance.ModelAnimation, scene->instances[i]);
		}
		ogt_vox_destroy_scene(scene);
	}
}