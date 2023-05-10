//#type compute
#version 460
#extension GL_EXT_shader_8bit_storage : enable
#extension GL_EXT_shader_explicit_arithmetic_types_int8 : enable

#include "Resources/Shaders/Includes/PBR.glsl"
#include "Resources/Shaders/Includes/Random.glsl"

#define TILE_SIZE 16
#define MAX_COLORS 1024
#define MAX_MODELS 1024

const uint OPAQUE = 255;

struct VoxelTopGrid
{
	uint  MaxTraverses;
	uint  CellsOffset;

	uint  Width;
	uint  Height;
	uint  Depth;
	float Size;

	uint Padding[2];
};

struct VoxelModel
{
	mat4  InverseTransform;

	uint  VoxelOffset;
	uint  Width;
	uint  Height;
	uint  Depth;
	uint  ColorIndex;
	uint  MaxTraverses;

	float VoxelSize;
	int   TopGridIndex;
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
	uint		 NumModels;
	VoxelModel	 Models[MAX_MODELS];
	VoxelTopGrid TopGrids[];
};

layout(std430, binding = 19) readonly buffer buffer_Colors
{		
	uint ColorPallete[MAX_COLORS][256];
};

layout(std430, binding = 20) readonly buffer buffer_TopGrids
{		
	uint8_t TopGridCells[];
};

layout(push_constant) uniform Uniforms
{
	uint Random;
	uint ModelIndex;
	uint WaterColorIndex;
	int Stage;
	int HeightStage;

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

bool IsWater(ivec3 index)
{
	uint width = Models[u_Uniforms.ModelIndex].Width;
	uint height = Models[u_Uniforms.ModelIndex].Height;
	uint depth = Models[u_Uniforms.ModelIndex].Depth;

	if (!IsValidVoxel(index, width, height, depth))
		return false;

	uint voxel = Index3D(index, width, height) + Models[u_Uniforms.ModelIndex].VoxelOffset;
	uint colorIndex = uint(Voxels[voxel]);
	return colorIndex == u_Uniforms.WaterColorIndex;
}


bool IsFilledVoxel(ivec3 index)
{
	uint width = Models[u_Uniforms.ModelIndex].Width;
	uint height = Models[u_Uniforms.ModelIndex].Height;
	uint depth = Models[u_Uniforms.ModelIndex].Depth;

	if (!IsValidVoxel(index, width, height, depth))
		return true;

	uint voxel = Index3D(index, width, height) + Models[u_Uniforms.ModelIndex].VoxelOffset;
	uint colorIndex = uint(Voxels[voxel]);
	uint color = ColorPallete[Models[u_Uniforms.ModelIndex].ColorIndex][colorIndex];
	uint alpha = VoxelAlpha(color);
	return alpha != 0;
}

bool TryMoveVoxel(ivec3 index, ivec3 newIndex, in VoxelModel model)
{
	if (!IsFilledVoxel(newIndex))
	{				
		uint voxelIndex = Index3D(index, model.Width, model.Height) + model.VoxelOffset;
		uint newVoxelIndex = Index3D(newIndex, model.Width, model.Height) + model.VoxelOffset;
		
		uint temp = uint(Voxels[voxelIndex]);
		Voxels[voxelIndex] = uint8_t(Voxels[newVoxelIndex]);
		Voxels[newVoxelIndex] = uint8_t(temp);
		
		return true;
	}
	return false;
}

const ivec2 stageOffsets[9] = {
		ivec2(0, 0),
		ivec2(1, 0),
		ivec2(1, 1),
		ivec2(0, 1),
		
		ivec2(1, 2),
		ivec2(2, 1),

		ivec2(2, 0),
		ivec2(2, 2),
		ivec2(0, 2)
	};

layout(local_size_x = TILE_SIZE, local_size_y = TILE_SIZE, local_size_z = 4) in;
void main() 
{
	VoxelModel model = Models[u_Uniforms.ModelIndex];

	ivec3 index = ivec3(int(gl_GlobalInvocationID.x), int(gl_GlobalInvocationID.z), int(gl_GlobalInvocationID.y));
	index.x = stageOffsets[u_Uniforms.Stage].x + index.x * 3;
	index.z = stageOffsets[u_Uniforms.Stage].y + index.z * 3;
	index.y = u_Uniforms.HeightStage + index.y * 2;

	if (IsWater(index))
	{
		ivec3 downIndex = ivec3(index.x, index.y - 1, index.z);
		ivec3 leftIndex = ivec3(index.x - 1, index.y, index.z);
		ivec3 rightIndex = ivec3(index.x + 1, index.y, index.z);
		ivec3 frontIndex = ivec3(index.x, index.y, index.z + 1);
		ivec3 backIndex = ivec3(index.x, index.y, index.z - 1);
		

		if (!TryMoveVoxel(index, downIndex, model))
		{
			if (u_Uniforms.Random != 0)
			{
				if (!TryMoveVoxel(index, leftIndex, model))
				{
					if (!TryMoveVoxel(index, frontIndex, model))
					{
						TryMoveVoxel(index, backIndex, model);					
					}
				}
			}
			else if (!TryMoveVoxel(index, rightIndex, model))
			{
				if (!TryMoveVoxel(index, frontIndex, model))
				{
					TryMoveVoxel(index, backIndex, model);
					
				}
			}			
		}	
	}




	//for (int y = 1 + u_Uniforms.HeightStage; y < model.Height; y += 2)
	//{
	//	ivec3 index = ivec3(int(gl_GlobalInvocationID.x), y, int(gl_GlobalInvocationID.y));
	//	index.x = stageOffsets[u_Uniforms.Stage].x + index.x * 3;
	//	index.z = stageOffsets[u_Uniforms.Stage].y + index.z * 3;
	//
	//	if (IsWater(index))
	//	{
	//		ivec3 downIndex = ivec3(index.x, index.y - 1, index.z);
	//		ivec3 leftIndex = ivec3(index.x - 1, index.y, index.z);
	//		ivec3 rightIndex = ivec3(index.x + 1, index.y, index.z);
	//		ivec3 frontIndex = ivec3(index.x, index.y, index.z + 1);
	//		ivec3 backIndex = ivec3(index.x, index.y, index.z - 1);
	//		
	//
	//		if (!TryMoveVoxel(index, downIndex, model))
	//		{
	//			if (u_Uniforms.Random != 0)
	//			{
	//				if (!TryMoveVoxel(index, leftIndex, model))
	//				{
	//					if (!TryMoveVoxel(index, frontIndex, model))
	//					{
	//						TryMoveVoxel(index, backIndex, model);					
	//					}
	//				}
	//			}
	//			else if (!TryMoveVoxel(index, rightIndex, model))
	//			{
	//				if (!TryMoveVoxel(index, frontIndex, model))
	//				{
	//					TryMoveVoxel(index, backIndex, model);
	//					
	//				}
	//			}			
	//		}	
	//	}
	//}
}