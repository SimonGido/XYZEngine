//#type compute
#version 460
#extension GL_EXT_shader_8bit_storage : enable
#extension GL_EXT_shader_explicit_arithmetic_types_int8 : enable

#include "Resources/Shaders/Includes/PBR.glsl"
#include "Resources/Shaders/Includes/Random.glsl"

#define TILE_SIZE 16
#define MAX_COLORS 1024

const uint OPAQUE = 255;


struct VoxelModel
{
	mat4  InverseModelView;
	mat4  Transform;
	vec4  RayOrigin;

	uint  VoxelOffset;
	uint  Width;
	uint  Height;
	uint  Depth;
	uint  ColorIndex;

	float VoxelSize;
	bool  OriginInside;

	uint Padding[1];
};

layout (std140, binding = 16) uniform Scene
{
	// Camera info
	mat4 u_InverseProjection;
	mat4 u_InverseView;	
	vec4 u_CameraPosition;
	vec4 u_ViewportSize;

	// Light info
	DirectionalLight u_DirectionalLight;
	uint MaxTraverse;
};


layout(std430, binding = 17) buffer buffer_Voxels
{		
	uint8_t Voxels[];
};

layout(std430, binding = 18) readonly buffer buffer_Models
{		
	uint NumModels;
	VoxelModel Models[];
};

layout(std430, binding = 19) readonly buffer buffer_Colors
{		
	uint ColorPallete[MAX_COLORS][256];
};

layout(push_constant) uniform Uniforms
{
	uint ModelIndex;
	uint CollisionModelIndex;
	uint RandSeed;
} u_Uniforms;


uint Index3D(int x, int y, int z, uint width, uint height)
{
	return x + width * (y + height * z);
}

uint Index3D(ivec3 index, uint width, uint height)
{
	return Index3D(index.x, index.y, index.z, width, height);
}

ivec3 GridScale(uint width0, uint height0, uint depth0, uint width1, uint height1, uint depth1)
{
	ivec3 scale = ivec3(
		width0 / width1,
		height0 / height1,
		depth0 / depth1
	);
	return scale;
}

uint VoxelAlpha(uint voxel)
{
	return bitfieldExtract(voxel, 24, 8);
}

bool IsValidVoxel(ivec3 voxel, uint width, uint height, uint depth)
{
	return ((voxel.x < width && voxel.x >= 0)
		 && (voxel.y < height && voxel.y >= 0)
		 && (voxel.z < depth && voxel.z >= 0));
}

bool IsFilledVoxel(ivec3 index, uint modelIndex)
{
	uint width = Models[modelIndex].Width;
	uint height = Models[modelIndex].Height;
	uint depth = Models[modelIndex].Depth;

	if (!IsValidVoxel(index, width, height, depth))
		return false;

	uint voxel = Index3D(index, width, height) + Models[modelIndex].VoxelOffset;
	uint colorIndex = uint(Voxels[voxel]);
	uint color = ColorPallete[Models[modelIndex].ColorIndex][colorIndex];
	uint alpha = VoxelAlpha(color);
	return alpha != 0;
}

layout(local_size_x = TILE_SIZE, local_size_y = TILE_SIZE, local_size_z = 1) in;
void main() 
{
	VoxelModel model = Models[u_Uniforms.ModelIndex];
	VoxelModel collisionModel = Models[u_Uniforms.CollisionModelIndex];
	
	uint modelVoxelOffset = Models[u_Uniforms.ModelIndex].VoxelOffset;
	uint collisionVoxelOffset = Models[u_Uniforms.CollisionModelIndex].VoxelOffset;

	ivec3 gridScale = GridScale(
		model.Width, model.Height, model.Depth,
		collisionModel.Width, collisionModel.Height, collisionModel.Depth
	);

	for (int y = 1; y < model.Height; y++)
	{
		ivec3 index = ivec3(int(gl_GlobalInvocationID.x), y, int(gl_GlobalInvocationID.y));
		if (IsFilledVoxel(index, u_Uniforms.ModelIndex))
		{
			ivec3 downIndex = ivec3(index.x, index.y - 1, index.z);
			if (!IsFilledVoxel(downIndex, u_Uniforms.ModelIndex))
			{
				ivec3 collisionIndex = downIndex / gridScale;
				if (!IsFilledVoxel(collisionIndex, u_Uniforms.CollisionModelIndex))
				{
					uint voxelIndex = Index3D(index, model.Width, model.Height) + modelVoxelOffset;
					uint downVoxelIndex = Index3D(downIndex, model.Width, model.Height) + modelVoxelOffset;
					
					uint temp = uint(Voxels[voxelIndex]);
					Voxels[voxelIndex] = uint8_t(Voxels[downVoxelIndex]);
					Voxels[downVoxelIndex] = uint8_t(temp);
				}
				else
				{
					// Reset snow to the top of the grid			
					float seed = float(u_Uniforms.RandSeed) * gl_GlobalInvocationID.x * gl_GlobalInvocationID.y;
					bool shouldReset = Random(0.0, 10.0, seed) > 9.8;
					if (shouldReset)
					{
						ivec3 upperIndex = ivec3(index.x, model.Height - 1, index.z);
						uint voxelIndex = Index3D(index, model.Width, model.Height) + modelVoxelOffset;
						uint upperVoxelIndex = Index3D(upperIndex, model.Width, model.Height) + modelVoxelOffset;
						
						uint8_t temp = Voxels[voxelIndex];
						Voxels[voxelIndex] = uint8_t(Voxels[upperVoxelIndex]);
						Voxels[upperVoxelIndex] = uint8_t(temp);
					}		
				}
			}
		}
	}
}