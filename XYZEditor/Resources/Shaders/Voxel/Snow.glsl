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
	uint  MaxTraverses;

	float VoxelSize;
	bool  OriginInside;
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
	uint SnowColorIndex;
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

bool IsSnow(ivec3 index)
{
	uint width = Models[u_Uniforms.ModelIndex].Width;
	uint height = Models[u_Uniforms.ModelIndex].Height;
	uint depth = Models[u_Uniforms.ModelIndex].Depth;

	if (!IsValidVoxel(index, width, height, depth))
		return false;

	uint voxel = Index3D(index, width, height) + Models[u_Uniforms.ModelIndex].VoxelOffset;
	uint colorIndex = uint(Voxels[voxel]);
	return colorIndex == u_Uniforms.SnowColorIndex;
}


bool IsFilledVoxel(ivec3 index)
{
	uint width = Models[u_Uniforms.ModelIndex].Width;
	uint height = Models[u_Uniforms.ModelIndex].Height;
	uint depth = Models[u_Uniforms.ModelIndex].Depth;

	if (!IsValidVoxel(index, width, height, depth))
		return false;

	uint voxel = Index3D(index, width, height) + Models[u_Uniforms.ModelIndex].VoxelOffset;
	uint colorIndex = uint(Voxels[voxel]);
	uint color = ColorPallete[Models[u_Uniforms.ModelIndex].ColorIndex][colorIndex];
	uint alpha = VoxelAlpha(color);
	return alpha != 0;
}

layout(local_size_x = TILE_SIZE, local_size_y = TILE_SIZE, local_size_z = 1) in;
void main() 
{
	VoxelModel model = Models[u_Uniforms.ModelIndex];

	uint modelVoxelOffset = Models[u_Uniforms.ModelIndex].VoxelOffset;

	for (int y = 1; y < model.Height; y++)
	{
		ivec3 index = ivec3(int(gl_GlobalInvocationID.x), y, int(gl_GlobalInvocationID.y));
		if (IsSnow(index))
		{
			ivec3 downIndex = ivec3(index.x, index.y - 1, index.z);
			if (!IsFilledVoxel(downIndex))
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
			
			break;
		}
	}
}