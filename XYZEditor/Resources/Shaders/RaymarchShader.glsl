//#type compute
#version 460
#extension GL_EXT_shader_8bit_storage : enable

#include "Resources/Shaders/Includes/Math.glsl"

const float FLT_MAX = 3.402823466e+38;

struct VoxelModel
{
	mat4  InverseTransform;
	uint  VoxelOffset;
	uint  Width;
	uint  Height;
	uint  Depth;
	float VoxelSize;

	uint Padding[3];
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

layout(std430, binding = 18) buffer buffer_Models
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


uint Index3D(int x, int y, int z, uint width, uint depth)
{
	return x + width * (y + depth * z);
}

uint Index3D(ivec3 index, uint width, uint depth)
{
	return Index3D(index.x, index.y, index.z, width, depth);
}


bool IsValidVoxel(ivec3 voxel, uint width, uint height, uint depth)
{
	return ((voxel.x < width && voxel.x > 0)
		 && (voxel.y < height && voxel.y > 0)
		 && (voxel.z < depth && voxel.z > 0));
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
	bool  Hit;
	float Depth;
};

struct RaymarchResult
{
	vec4  Color;
	float Distance;
	bool  OpaqueHit;
	bool  Hit;
};

RaymarchHitResult RayMarch(vec3 origin, vec3 t_max, vec3 t_delta, ivec3 current_voxel, ivec3 step, uint maxTraverses, int modelIndex, float currentDepth)
{
	RaymarchHitResult result;
	result.Hit = false;

	uint width = Models[modelIndex].Width;
	uint height = Models[modelIndex].Height;
	uint depth = Models[modelIndex].Depth;
	uint voxelOffset = Models[modelIndex].VoxelOffset;

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

		float newDepth = distance(origin, t_max);
		result.Depth = newDepth;
		if (newDepth > currentDepth)
			break;

		if (IsValidVoxel(current_voxel, width, height, depth))
		{
			uint voxelIndex = Index3D(current_voxel, width, depth) + voxelOffset;
			uint colorIndex = uint(Voxels[voxelIndex]);
			uint voxel = Colors[colorIndex];

			if (voxel != 0)
			{
				float light = dot(-u_LightDirection.xyz, normal);
				vec4 voxelColor = VoxelToColor(voxel);
				vec3 color = voxelColor.rgb * u_LightColor.rgb * light;
				result.Alpha = VoxelAlpha(voxel);
				result.Color = vec4(color.rgb, voxelColor.a);
				result.Hit = true;				
				break;
			}
		}
	}
	result.TraverseCount = i;
	result.Max = t_max;
	result.CurrentVoxel = current_voxel;

	return result;
}

RaymarchResult RayMarch(vec3 origin, vec3 direction, int modelIndex, float currentDepth)
{
	RaymarchResult result;
	result.OpaqueHit = false;
	result.Hit = false;
	result.Distance = FLT_MAX;
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
	
	// Raymarch until we find first hit to determine default color
	RaymarchHitResult hitResult = RayMarch(origin, t_max, t_delta, current_voxel, step, remainingTraverses, modelIndex, currentDepth);
	float newDepth = distance(origin, hitResult.Max);
	if (newDepth > currentDepth) // Depth test
		return result;

	// If we hit something it is our default color
	if (hitResult.Hit)
	{
		result.Color = hitResult.Color;
		result.Hit = true;
		result.OpaqueHit = hitResult.Alpha == 1;
		result.Distance = newDepth;
	}
	
	// Continue raymarching until we hit opaque object or we are out of traverses
	remainingTraverses -= hitResult.TraverseCount;
	while (remainingTraverses != 0)
	{		
		if (result.OpaqueHit) // Opaque hit => stop raymarching
			break;

		hitResult = RayMarch(origin, hitResult.Max, t_delta, hitResult.CurrentVoxel, step, remainingTraverses, modelIndex, currentDepth);	
		newDepth = distance(origin, hitResult.Max); // Store raymarch distance
		if (newDepth > currentDepth) // if new depth is bigger than currentDepth it means there is something in front of us
			break;
		
		// We passed depth test
		if (hitResult.Hit) // We hit something so mix colors together
		{
			result.Color.rgb = mix(result.Color.rgb, hitResult.Color.rgb, 1.0 - result.Color.a);
			result.Hit = true;
			result.OpaqueHit = hitResult.Alpha == 1;
		}
		
		remainingTraverses -= hitResult.TraverseCount;
	}
	result.Distance = newDepth;
	return result;
}



layout(local_size_x = 32, local_size_y = 32, local_size_z = 1) in;
void main() 
{
	for (int i = 0; i < NumModels; i++)
	{
		float currentDepth = imageLoad(o_DepthImage, ivec2(gl_GlobalInvocationID)).r;
		Ray ray = CreateRay(gl_GlobalInvocationID.xy, i);
		RaymarchResult result = RayMarch(ray.Origin, ray.Direction, i, currentDepth);
				

		if (result.Hit)
		{
			if (!result.OpaqueHit)
			{
				vec4 originalColor = imageLoad(o_Image, ivec2(gl_GlobalInvocationID));
				result.Color.rgb = mix(result.Color.rgb, originalColor.rgb, 1.0 - result.Color.a);
			}
			else // We store depth only if we hit something opaque
			{
				imageStore(o_DepthImage, ivec2(gl_GlobalInvocationID), vec4(result.Distance, 0,0,0)); // Store new depth
			}
			imageStore(o_Image, ivec2(gl_GlobalInvocationID), result.Color); // Store color				
		}
	}
}