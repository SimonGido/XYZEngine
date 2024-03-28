//#type compute
#version 460
#extension GL_EXT_shader_8bit_storage : enable

#define TILE_SIZE 16

struct VoxelCompressedCell
{
	uint VoxelCount;
	uint VoxelOffset;
};

layout(push_constant) uniform Uniforms
{
	uint Width;
	uint Height;
	uint Depth;
	uint Scale;

} u_Uniforms;

layout(std430, binding = 17) readonly buffer buffer_Voxels
{		
	uint8_t Voxels[];
};


layout(std430, binding = 20) buffer buffer_Compressed
{		
	VoxelCompressedCell CompressedCells[];
};

layout(std430, binding = 24) buffer buffer_UncompressedVoxels
{		
	uint8_t UncompressedVoxels[];
};


uint Index3D(int x, int y, int z, uint width, uint height)
{
	return x + width * (y + height * z);
}

uint Index3D(ivec3 index, uint width, uint height)
{
	return Index3D(index.x, index.y, index.z, width, height);
}


layout(local_size_x = TILE_SIZE, local_size_y = TILE_SIZE, local_size_z = 1) in;
void main() 
{	
	ivec2 location = ivec2(gl_GlobalInvocationID.xy);

	
}