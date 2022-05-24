//#type compute
#version 460



struct Voxel
{
	uint Color;
	uint Distance;
};


// Voxel colors
layout(std430, binding = 3) buffer buffer_Voxels
{
	Voxel Voxels[];
};

layout(std140, binding = 0) uniform Scene
{
	mat4 u_ViewProjection;
	mat4 u_InverseView;
	mat4 u_CameraFrustum;
	vec4 u_CameraPosition;
	vec4 u_LightDirection;
	vec4 u_LightColor;
	vec4 u_BackgroundColor;

	//Voxel chunk info
	vec4  u_ChunkPosition;
	uint  u_MaxTraverse;
	uint  u_Width;
	uint  u_Height;
	uint  u_Depth;
	float u_VoxelSize;
	float u_Padding[3];
};

layout(local_size_x = 32, local_size_y = 32, local_size_z = 1) in;
void main(void)
{
	uint id = gl_GlobalInvocationID.x + gl_GlobalInvocationID.y * gl_NumWorkGroups.x * gl_WorkGroupSize.x;
	if (id >= u_Width * u_Height * u_Depth)
		return;

	for (uint x = 0; x < u_Width; ++x)
	{
		for (uint y = 0; y < u_Height; ++y)
		{
			for (uint z = 0; z < u_Depth; ++z)
			{
				//if (x + y + z != id)
				//	Voxels[id].Distance = atomicMin(Voxels[id].Distance, x + y + z);
			}
		}
	}
}