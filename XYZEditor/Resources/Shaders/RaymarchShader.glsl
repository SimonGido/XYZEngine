//#type compute
#version 460
#extension GL_EXT_shader_8bit_storage : enable

#include "Resources/Shaders/Includes/Math.glsl"


const uint SCENE_WIDTH  = 512;
const uint SCENE_HEIGHT = 512;
const uint SCENE_DEPTH  = 512;

struct VoxelModel
{
	mat4  InverseTransform;
	uint  FirstVoxel;
	uint  LastVoxel;
	float VoxelSize;

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
	vec4 u_LightDirection;
	vec4 u_LightColor;
	uint MaxTraverse;
};


layout(std430, binding = 17) buffer buffer_Voxels
{	
	uint Colors[256];
	uint8_t Voxels[];
};

layout(std140, binding = 18) buffer buffer_Models
{	
	uint NumModels;
	VoxelModel Models[];
};


layout(binding = 19, rgba32f) uniform image2D o_Image;
layout(binding = 20, r32f) uniform image2D o_DepthImage;

struct Ray
{
	vec3 Origin;
	vec3 Direction;
};

Ray CreateRay(vec2 coords, int modelIndex)
{
	coords.x /= u_ViewportSize.x;
	coords.y /= u_ViewportSize.y;
	coords = coords * 2.0 - 1.0; // -1 -> 1
	vec4 target = u_InverseProjection * vec4(coords.x, -coords.y, 1, 1);
	vec4 rayOrigin = u_CameraPosition;
	
	Ray ray;
	ray.Origin = (Models[modelIndex].InverseTransform * rayOrigin).xyz;

	ray.Direction = vec3(Models[modelIndex].InverseTransform * u_InverseView * vec4(normalize(vec3(target) / target.w), 0)); // World space
	ray.Direction = normalize(ray.Direction);

	return ray;
}


uint Index3D(int x, int y, int z)
{
	return x + SCENE_WIDTH * (y + SCENE_DEPTH * z);
}

uint Index3D(ivec3 index)
{
	return Index3D(index.x, index.y, index.z);
}


bool IsValidVoxel(ivec3 voxel)
{
	return ((voxel.x < SCENE_WIDTH  && voxel.x > 0)
		 && (voxel.y < SCENE_HEIGHT && voxel.y > 0)
		 && (voxel.z < SCENE_DEPTH  && voxel.z > 0));
}


vec4 VoxelToColor(uint voxel)
{
	vec4 color;
	color.x = bitfieldExtract(voxel, 0, 8) / 255.0;
	color.y = bitfieldExtract(voxel, 8, 8) / 255.0;
	color.z = bitfieldExtract(voxel, 16, 8) / 255.0;
	color.w = bitfieldExtract(voxel, 24, 8) / 255.0;

	return color;
}
uint VoxelAlpha(uint voxel)
{
	return bitfieldExtract(voxel, 24, 8) / 255;
}

struct RaymarchHitResult
{
	vec4  Color;
	vec3  Max;
	ivec3 CurrentVoxel;
	uint  Alpha;
	uint  TraverseCount;
};

struct RaymarchResult
{
	vec4 Color;
	float Distance;
};

RaymarchHitResult RayMarch(vec3 t_max, vec3 t_delta, ivec3 current_voxel, ivec3 step, uint maxTraverses, int modelIndex)
{
	RaymarchHitResult result;
	result.Color = vec4(0.3, 0.2, 0.7, 1.0);
	uint i = 0;
	for (i = 0; i < maxTraverses; i++)
	{
		vec3 normal;
		if (t_max.x < t_max.y && t_max.x < t_max.z) 
		{
			normal = vec3(float(-step.x), 0.0, 0.0);
			t_max.x += t_delta.x;
			current_voxel.x += step.x;
		}
		else if (t_max.y < t_max.z) 
		{
			normal = vec3(0.0, float(-step.y), 0.0);
			t_max.y += t_delta.y;
			current_voxel.y += step.y;			
		}
		else 
		{
			normal = vec3(0.0, 0.0, float(-step.z));
			t_max.z += t_delta.z;
			current_voxel.z += step.z;		
		}
		if (IsValidVoxel(current_voxel))
		{
			uint index = uint(Voxels[Index3D(current_voxel)]);
			// Is in model voxel range
			//if (index >= Models[modelIndex].FirstVoxel && index < Models[modelIndex].LastVoxel)
			{
				uint voxel = Colors[index];
				if (voxel != 0)
				{
					float light = dot(-u_LightDirection.xyz, normal);
					vec4 voxelColor = VoxelToColor(voxel);
					vec3 color = voxelColor.rgb * u_LightColor.rgb * light;
					result.Alpha = VoxelAlpha(voxel);
					result.Color = vec4(color.rgb, voxelColor.a);
					break;
				}
			}
		}
	}
	result.TraverseCount = i;
	result.Max = t_max;
	result.CurrentVoxel = current_voxel;

	return result;
}

RaymarchResult RayMarch(vec3 origin, vec3 direction, int modelIndex)
{
	RaymarchResult result;
	vec4 defaultColor = vec4(0.3, 0.2, 0.7, 1.0);
	ivec3 current_voxel = ivec3(floor(origin / Models[modelIndex].VoxelSize));
	
	ivec3 step = ivec3(
		(direction.x > 0.0) ? 1 : -1,
		(direction.y > 0.0) ? 1 : -1,
		(direction.z > 0.0) ? 1 : -1
	);
	vec3 next_boundary = vec3(
		float((step.x > 0) ? current_voxel.x + 1 : current_voxel.x) * Models[modelIndex].VoxelSize,
		float((step.y > 0) ? current_voxel.y + 1 : current_voxel.y) * Models[modelIndex].VoxelSize,
		float((step.z > 0) ? current_voxel.z + 1 : current_voxel.z) * Models[modelIndex].VoxelSize
	);

	vec3 t_max = (next_boundary - origin) / direction; // we will move along the axis with the smallest value
	vec3 t_delta = Models[modelIndex].VoxelSize / direction * vec3(step);	


	uint remainingTraverses = MaxTraverse;
	RaymarchHitResult hit = RayMarch(t_max, t_delta, current_voxel, step, remainingTraverses, modelIndex);
	result.Color = hit.Color;
	remainingTraverses -= hit.TraverseCount;

	bool opaqueHit = false;
	while (remainingTraverses != 0)
	{
		if (hit.Alpha == 1)
		{
			opaqueHit = true;
			break;
		}
		hit = RayMarch(hit.Max, t_delta, hit.CurrentVoxel, step, remainingTraverses, modelIndex);
		result.Color.rgb = mix(result.Color.rgb, hit.Color.rgb, 1.0 - result.Color.a);
		remainingTraverses -= hit.TraverseCount;
	}

	if (!opaqueHit) // We did not hit opaque voxel => blend with background color
	{
		result.Color.rgb = mix(result.Color.rgb, defaultColor.rgb, 1.0 - result.Color.a);
	}

	result.Distance = distance(origin, hit.Max);
	return result;
}



layout(local_size_x = 32, local_size_y = 32, local_size_z = 1) in;
void main() 
{
	for (int i = 0; i < NumModels; i++)
	{
		Ray ray = CreateRay(gl_GlobalInvocationID.xy, i);
		RaymarchResult result = RayMarch(ray.Origin, ray.Direction, i);
		
		//float currentDistance = imageLoad(o_DepthImage, ivec2(gl_GlobalInvocationID)).r;
		//if (result.Distance < currentDistance)
			imageStore(o_Image, ivec2(gl_GlobalInvocationID), result.Color);
	}
}