//#type compute
#version 460
#extension GL_EXT_shader_8bit_storage : enable

#include "Resources/Shaders/Includes/Math.glsl"
#include "Resources/Shaders/Includes/PBR.glsl"

#define TILE_SIZE 16
#define MAX_COLORS 1024

const float FLT_MAX = 3.402823466e+38;
const float EPSILON = 0.01;
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


layout(std430, binding = 17) readonly buffer buffer_Voxels
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

layout(binding = 20, rgba32f) uniform image2D o_Image;
layout(binding = 21, r32f) uniform image2D o_DepthImage;

struct Ray
{
	vec3 Origin;
	vec3 Direction;
};


Ray CreateRay(vec2 coords, uint modelIndex)
{
	coords.x /= u_ViewportSize.x;
	coords.y /= u_ViewportSize.y;
	coords = coords * 2.0 - 1.0; // -1 -> 1
	vec4 target = u_InverseProjection * vec4(coords.x, -coords.y, 1, 1);
	Ray ray;
	ray.Origin = Models[modelIndex].RayOrigin.xyz;

	ray.Direction = vec3(Models[modelIndex].InverseModelView * vec4(normalize(target.xyz / target.w), 0)); // World space
	ray.Direction = normalize(ray.Direction) + EPSILON;

	return ray;
}

Ray CreateCameraRay(vec2 coords)
{
	coords.x /= u_ViewportSize.x;
	coords.y /= u_ViewportSize.y;
	coords = coords * 2.0 - 1.0; // -1 -> 1
	vec4 target = u_InverseProjection * vec4(coords.x, -coords.y, 1, 1);
	Ray ray;
	ray.Origin = u_CameraPosition.xyz;

	ray.Direction = vec3(u_InverseView * vec4(normalize(vec3(target) / target.w), 0)); // World space
	ray.Direction = normalize(ray.Direction) + EPSILON;

	return ray;
}

vec3 VoxelWorldPosition(ivec3 voxel, uint modelIndex)
{
	float voxelSize = Models[modelIndex].VoxelSize;
	vec3 localPosition = vec3(voxel) * voxelSize + voxelSize / 2.0;

	mat4 transform = inverse(Models[modelIndex].Transform);

	return (transform * vec4(localPosition, 1.0)).xyz;
}

uint Index3D(int x, int y, int z, uint width, uint height)
{
	return x + width * (y + height * z);
}

uint Index3D(ivec3 index, uint width, uint height)
{
	return Index3D(index.x, index.y, index.z, width, height);
}


bool IsValidVoxel(ivec3 voxel, uint width, uint height, uint depth)
{
	return ((voxel.x < width && voxel.x >= 0)
		 && (voxel.y < height && voxel.y >= 0)
		 && (voxel.z < depth && voxel.z >= 0));
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
	return bitfieldExtract(voxel, 24, 8);
}

float VoxelDistanceFromRay(vec3 origin, vec3 direction, ivec3 voxel, float voxelSize)
{
	vec3 boxMin = vec3(voxel.x * voxelSize, voxel.y * voxelSize, voxel.z * voxelSize);
	vec3 boxMax = boxMin + voxelSize;
	float dist;
	if (RayBoxIntersection(origin, direction, boxMin, boxMax, dist))
		return dist;
	else
		return FLT_MAX;
}

bool DistanceTest(Ray ray, ivec3 voxel, float voxelSize, float currentDistance, out float newDistance)
{
	newDistance = VoxelDistanceFromRay(ray.Origin,ray.Direction, voxel, voxelSize);		
	return newDistance < currentDistance;
}

struct RaymarchHitResult
{
	vec4  Color;
	vec3  T_Max;
	vec3  Normal;
	ivec3 CurrentVoxel;
	uint  Alpha;
	uint  TraverseCount;
	bool  Hit;
	float Distance;
};

struct RaymarchResult
{
	vec4  Color;
	vec3  T_Max;
	vec3  Normal;
	ivec3 CurrentVoxel;
	float Distance;
	bool  OpaqueHit;
	bool  Hit;
};

RaymarchHitResult RayMarch(Ray ray, vec3 t_max, vec3 t_delta, ivec3 current_voxel, ivec3 step, uint maxTraverses, uint modelIndex, float currentDistance)
{
	RaymarchHitResult result;
	result.Hit = false;

	uint width = Models[modelIndex].Width;
	uint height = Models[modelIndex].Height;
	uint depth = Models[modelIndex].Depth;
	uint voxelOffset = Models[modelIndex].VoxelOffset;
	uint colorPalleteIndex = Models[modelIndex].ColorIndex;
	float voxelSize = Models[modelIndex].VoxelSize;
	
	uint i = 0;
	for (i = 0; i < maxTraverses; i++)
	{
		if (t_max.x < t_max.y && t_max.x < t_max.z) 
		{
			result.Normal = vec3(float(-step.x), 0.0, 0.0);
			t_max.x += t_delta.x;
			current_voxel.x += step.x;
		}
		else if (t_max.y < t_max.z) 
		{
			result.Normal = vec3(0.0, float(-step.y), 0.0);
			t_max.y += t_delta.y;
			current_voxel.y += step.y;			
		}
		else 
		{
			result.Normal = vec3(0.0, 0.0, float(-step.z));
			t_max.z += t_delta.z;
			current_voxel.z += step.z;		
		}

		if (IsValidVoxel(current_voxel, width, height, depth))
		{
			if (!DistanceTest(ray, current_voxel, voxelSize, currentDistance, result.Distance))
				break;

			uint voxelIndex = Index3D(current_voxel, width, height) + voxelOffset;
			uint colorIndex = uint(Voxels[voxelIndex]);
			uint voxel = ColorPallete[colorPalleteIndex][colorIndex];

			if (voxel != 0)
			{
				result.Color = VoxelToColor(voxel);
				result.Alpha = VoxelAlpha(voxel);
				result.Hit = true;				
				break;
			}
		}
	}
	result.TraverseCount = i;
	result.T_Max = t_max;
	result.CurrentVoxel = current_voxel;

	return result;
}

RaymarchResult RayMarch(Ray ray, vec3 origin, vec3 direction, uint modelIndex, float currentDistance)
{
	RaymarchResult result;
	result.OpaqueHit = false;
	result.Hit = false;
	result.Distance = FLT_MAX;
		
	float voxelSize = Models[modelIndex].VoxelSize;
	
	ivec3 current_voxel = ivec3(floor(origin / voxelSize));
	
	ivec3 step = ivec3(
		(direction.x > 0.0) ? 1 : -1,
		(direction.y > 0.0) ? 1 : -1,
		(direction.z > 0.0) ? 1 : -1
	);
	vec3 next_boundary = vec3(
		float((step.x > 0) ? current_voxel.x + 1 : current_voxel.x) * voxelSize,
		float((step.y > 0) ? current_voxel.y + 1 : current_voxel.y) * voxelSize,
		float((step.z > 0) ? current_voxel.z + 1 : current_voxel.z) * voxelSize
	);
	

	vec3 t_max = (next_boundary - origin) / direction; // we will move along the axis with the smallest value
	vec3 t_delta = voxelSize / direction * vec3(step);	


	uint remainingTraverses = Models[modelIndex].MaxTraverses;
	
	// Raymarch until we find first hit to determine default color
	RaymarchHitResult hitResult = RayMarch(ray, t_max, t_delta, current_voxel, step, remainingTraverses, modelIndex, currentDistance);	
	remainingTraverses -= hitResult.TraverseCount;


	float newDistance = hitResult.Distance;
	if (newDistance > currentDistance) // Depth test
		return result;

	// If we hit something it is our default color
	if (hitResult.Hit)
	{
		result.Color = hitResult.Color;
		result.Hit = true;
		result.OpaqueHit = hitResult.Alpha == OPAQUE;
		result.Distance = newDistance;
		result.Normal = hitResult.Normal;
		result.CurrentVoxel = hitResult.CurrentVoxel;
		result.T_Max = hitResult.T_Max;
	}
	
	// Continue raymarching until we hit opaque object or we are out of traverses
	while (remainingTraverses != 0)
	{		
		if (result.OpaqueHit) // Opaque hit => stop raymarching
			break;

		hitResult = RayMarch(ray, hitResult.T_Max, t_delta, hitResult.CurrentVoxel, step, remainingTraverses, modelIndex, currentDistance);	
		newDistance = hitResult.Distance; // Store raymarch distance
		if (newDistance > currentDistance) // if new depth is bigger than currentDepth it means there is something in front of us
			break;
		
		// We passed depth test
		if (hitResult.Hit) // We hit something so mix colors together
		{
			result.Color.rgb = mix(result.Color.rgb, hitResult.Color.rgb, 1.0 - result.Color.a);
			result.Hit = true;
			result.OpaqueHit = hitResult.Alpha == OPAQUE;
			result.Normal = hitResult.Normal;
			result.CurrentVoxel = hitResult.CurrentVoxel;
			result.T_Max = hitResult.T_Max;
		}
		
		remainingTraverses -= hitResult.TraverseCount;
	}
	result.Distance = newDistance;
	return result;
}

bool ResolveRayModelIntersection(inout vec3 origin, vec3 direction, uint modelIndex)
{
	bool result = Models[modelIndex].OriginInside; // Default is true in case origin is inside
	if (!result)
	{
		float width  = float(Models[modelIndex].Width);
		float height = float(Models[modelIndex].Height);
		float depth  = float(Models[modelIndex].Depth);
		float voxelSize = Models[modelIndex].VoxelSize;
	
		vec3 boxMin = vec3(0,0,0);
		vec3 boxMax = vec3(width, height, depth) * voxelSize;
		// Check if we are intersecting with grid
		float dist;
		result = RayBoxIntersection(origin, direction, boxMin, boxMax, dist);
		origin = origin + direction * (dist - EPSILON); // Move origin to first intersection
	}
	return result;
}

bool ValidPixel(ivec2 index)
{
	return index.x <= int(u_ViewportSize.x) && index.y <= int(u_ViewportSize.y);
}

// Constant normal incidence Fresnel factor for all dielectrics.
const vec3 Fdielectric = vec3(0.04);

vec3 CalculateDirLights(vec3 voxelPosition, vec3 albedo, vec3 normal)
{
	PBRParameters pbr;
	pbr.Roughness = 0.8;
	pbr.Metalness = 0.2;
	
	pbr.Normal = normal;
	pbr.View = normalize(u_CameraPosition.xyz - voxelPosition);
	pbr.NdotV = max(dot(pbr.Normal, pbr.View), 0.0);
	pbr.Albedo = albedo;

	vec3 F0 = mix(Fdielectric, pbr.Albedo, pbr.Metalness);
	return CalculateDirLight(F0, u_DirectionalLight, pbr);
}


layout(local_size_x = TILE_SIZE, local_size_y = TILE_SIZE, local_size_z = 1) in;
void main() 
{
	ivec2 textureIndex = ivec2(gl_GlobalInvocationID.xy);
	for (uint i = 0; i < NumModels; i++)
	{
		Ray ray = CreateRay(textureIndex, i);
		vec3 origin = ray.Origin;
		vec3 direction = ray.Direction;

		// Check if ray intersects with model and move origin of ray
		if (!ResolveRayModelIntersection(origin, direction, i))
			continue;

		float currentDistance = imageLoad(o_DepthImage, textureIndex).r;
		RaymarchResult result = RayMarch(ray, origin, direction, i, currentDistance);

		if (result.Hit)
		{			
			if (!result.OpaqueHit)
			{
				vec4 originalColor = imageLoad(o_Image, textureIndex);
				result.Color.rgb = mix(result.Color.rgb, originalColor.rgb, 1.0 - result.Color.a);
			}
			else // Store depth only for opaque hits
			{
				imageStore(o_DepthImage, textureIndex, vec4(result.Distance, 0,0,0)); // Store new depth
			}

			vec4 worldHitPosition = Models[i].Transform * vec4(result.T_Max, 1.0);
			vec3 worldNormal = mat3(Models[i].Transform) * -result.Normal;

			result.Color.rgb = CalculateDirLights(worldHitPosition.xyz, result.Color.rgb, worldNormal);

			imageStore(o_Image, textureIndex, result.Color); // Store color		
		}
	}
}