//#type compute
#version 460
#extension GL_EXT_shader_8bit_storage : enable

#include "Resources/Shaders/Includes/Math.glsl"
#include "Resources/Shaders/Includes/PBR.glsl"

#define TILE_SIZE 16
#define MAX_COLORS 1024
#define MAX_MODELS 1024

const float FLT_MAX = 3.402823466e+38;
const float EPSILON = 0.01;
const uint OPAQUE = 255;

layout(push_constant) uniform Uniforms
{
	bool UseTopGrid;
} u_Uniforms;

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


layout(std430, binding = 17) readonly buffer buffer_Voxels
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

layout(binding = 21, rgba16f) uniform image2D o_Image;
layout(binding = 22, r32f) uniform image2D o_DepthImage;

struct Ray
{
	vec3 Origin;
	vec3 Direction;
};

struct AABB
{
	vec3 Min;
	vec3 Max;
};


AABB VoxelAABB(ivec3 voxel, float voxelSize)
{
	AABB result;
	result.Min = vec3(voxel.x * voxelSize, voxel.y * voxelSize, voxel.z * voxelSize);
	result.Max = result.Min + voxelSize;
	return result;
}

AABB ModelAABB(in VoxelModel model)
{
	AABB result;
	result.Min = vec3(0.0);
	result.Max = vec3(model.Width, model.Height, model.Depth) * model.VoxelSize;
	return result;
}

Ray CreateRay(vec3 origin, in mat4 inverseModelSpace, vec2 coords)
{
	coords.x /= u_ViewportSize.x;
	coords.y /= u_ViewportSize.y;
	coords = coords * 2.0 - 1.0; // -1 -> 1
	vec4 target = u_InverseProjection * vec4(coords.x, -coords.y, 1, 1);
	Ray ray;
	ray.Origin = (inverseModelSpace * vec4(origin, 1.0)).xyz;

	ray.Direction = vec3(inverseModelSpace * u_InverseView * vec4(normalize(target.xyz / target.w), 0)); // World space
	ray.Direction = normalize(ray.Direction) + EPSILON;

	return ray;
}

Ray CreateRay(vec3 origin, vec2 coords)
{
	coords.x /= u_ViewportSize.x;
	coords.y /= u_ViewportSize.y;
	coords = coords * 2.0 - 1.0; // -1 -> 1
	vec4 target = u_InverseProjection * vec4(coords.x, -coords.y, 1, 1);
	Ray ray;
	ray.Origin = origin;

	ray.Direction = vec3(u_InverseView * vec4(normalize(vec3(target) / target.w), 0)); // World space
	ray.Direction = normalize(ray.Direction) + EPSILON;

	return ray;
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
	AABB aabb = VoxelAABB(voxel, voxelSize);
	if (PointInBox(origin, aabb.Min, aabb.Max))
		return 0.0;

	float dist;
	if (RayBoxIntersection(origin, direction, aabb.Min, aabb.Max, dist))
		return dist;
		
	return FLT_MAX;
}


struct RaymarchResult
{
	vec4  Color;
	vec3  Normal;
	float Distance;
	bool  OpaqueHit;
	bool  Hit;
};


struct RaymarchState
{
	vec4  Color;
	vec3  Max;
	vec3  Normal;
	ivec3 CurrentVoxel;
	uint  Alpha;
	uint  Traverses;
	bool  Hit;
	float Distance;
};


void PerformStep(inout RaymarchState state, ivec3 step, vec3 delta)
{
	if (state.Max.x < state.Max.y && state.Max.x < state.Max.z) 
	{
		state.Normal = vec3(float(-step.x), 0.0, 0.0);
		state.Max.x += delta.x;
		state.CurrentVoxel.x += step.x;
	}
	else if (state.Max.y < state.Max.z) 
	{
		state.Normal = vec3(0.0, float(-step.y), 0.0);
		state.Max.y += delta.y;
		state.CurrentVoxel.y += step.y;			
	}
	else 
	{
		state.Normal = vec3(0.0, 0.0, float(-step.z));
		state.Max.z += delta.z;
		state.CurrentVoxel.z += step.z;		
	}	
}

void RayMarch(in Ray ray, inout RaymarchState state, vec3 delta, ivec3 step, in VoxelModel model, float currentDistance)
{
	state.Hit = false;
	state.Color = vec4(0,0,0,0);
	state.Alpha = 0;

	uint width = model.Width;
	uint height = model.Height;
	uint depth = model.Depth;
	uint voxelOffset = model.VoxelOffset;
	uint colorPalleteIndex = model.ColorIndex;
	float voxelSize = model.VoxelSize;

	while (state.Traverses != 0)
	{
		state.Traverses -= 1;
		if (state.Distance > currentDistance)
			break;

		if (IsValidVoxel(state.CurrentVoxel, width, height, depth))
		{
			uint voxelIndex = Index3D(state.CurrentVoxel, width, height) + voxelOffset;
			uint colorIndex = uint(Voxels[voxelIndex]);
			uint voxel = ColorPallete[colorPalleteIndex][colorIndex];

			if (voxel != 0)
			{
				state.Color = VoxelToColor(voxel);
				state.Alpha = VoxelAlpha(voxel);
				state.Hit = true;	
				break;
			}
		}

		PerformStep(state, step, delta);
		state.Distance += voxelSize * length(ray.Direction);
	}
}

RaymarchState CreateRaymarchState(in Ray ray, vec3 origin, vec3 direction, ivec3 step, uint maxTraverses, float voxelSize)
{
	RaymarchState state;
	state.Distance = distance(ray.Origin, origin);
	state.CurrentVoxel = ivec3(floor(origin / voxelSize));
	state.Traverses = maxTraverses;

	vec3 next_boundary = vec3(
		float((step.x > 0) ? state.CurrentVoxel.x + 1 : state.CurrentVoxel.x) * voxelSize,
		float((step.y > 0) ? state.CurrentVoxel.y + 1 : state.CurrentVoxel.y) * voxelSize,
		float((step.z > 0) ? state.CurrentVoxel.z + 1 : state.CurrentVoxel.z) * voxelSize
	);

	state.Max = (next_boundary - origin) / direction; // we will move along the axis with the smallest value
	return state;
}

RaymarchResult RayMarch(in Ray ray, vec3 origin, vec3 direction, in VoxelModel model, float currentDistance, uint maxTraverses)
{
	RaymarchResult result;
	result.OpaqueHit = false;
	result.Hit = false;
	result.Distance = 0;
			
	ivec3 step = ivec3(
		(direction.x > 0.0) ? 1 : -1,
		(direction.y > 0.0) ? 1 : -1,
		(direction.z > 0.0) ? 1 : -1
	);
			
	float voxelSize = model.VoxelSize;
	vec3 delta = voxelSize / direction * vec3(step);	

	RaymarchState state = CreateRaymarchState(ray, origin, direction, step, maxTraverses, voxelSize);

	// Raymarch until we find first hit to determine default color
	RayMarch(ray, state, delta, step, model, currentDistance);
	
	
	// If we hit something it is our default color
	if (state.Hit)
	{
		result.Color = state.Color;
		result.Hit = true;
		result.OpaqueHit = state.Alpha == OPAQUE;
		result.Distance = state.Distance;
		result.Normal = state.Normal;

		//if (!result.OpaqueHit)
		//	state.Traverses = model.MaxTraverses; // Hit was not opaque reset traverses for better visual results
	}

	if (state.Distance > currentDistance) // Depth test
		return result;
	
	// Continue raymarching until we hit opaque object or we are out of traverses
	while (state.Traverses != 0)
	{		
		if (result.OpaqueHit) // Opaque hit => stop raymarching
			break;
		
		PerformStep(state, step, delta); // Hit was not opaque we continue raymarching, perform step to get out transparent voxel
		RayMarch(ray, state, delta, step, model, currentDistance);
		if (state.Distance > currentDistance) // if new depth is bigger than currentDepth it means there is something in front of us
			break;
		
		// We passed depth test
		if (state.Hit) // We hit something so mix colors together
		{
			result.Color.rgb = mix(result.Color.rgb, state.Color.rgb, state.Color.a);
			result.Color.a = mix(result.Color.a, state.Color.a, result.Color.a); // Increase alpha of our current result
			result.Hit = true;
			result.OpaqueHit = result.Color.a >= 1.0;
			result.Normal = state.Normal;
		}
	}
	result.Distance = state.Distance;
	return result;
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



RaymarchResult RaymarchTopGrid(in Ray ray, vec3 origin, vec3 direction, in VoxelModel model, float currentDistance, in VoxelTopGrid grid)
{
	RaymarchResult result;
	result.Hit = false;

	ivec2 textureIndex = ivec2(gl_GlobalInvocationID.xy);
	float voxelSize = grid.Size;
	
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

	for (uint i = 0; i < grid.MaxTraverses; i++)
	{
		if (IsValidVoxel(current_voxel, grid.Width, grid.Height, grid.Depth))
		{
			uint voxelIndex = Index3D(current_voxel, grid.Width, grid.Height) + grid.CellsOffset;
			if (uint(TopGridCells[voxelIndex]) != 0)
			{			
				float dist = VoxelDistanceFromRay(origin, direction, current_voxel, voxelSize);
				origin += direction * (dist - EPSILON); // Move origin to hit of top grid cell
							
				// Raymarch from new origin
				result = RayMarch(ray, origin, direction, model, currentDistance, grid.MaxTraverses);
				if (result.Hit)
					return result;
			}
		}

		if (t_max.x < t_max.y && t_max.x < t_max.z) 
		{
			t_max.x += t_delta.x;
			current_voxel.x += step.x;
		}
		else if (t_max.y < t_max.z) 
		{
			t_max.y += t_delta.y;
			current_voxel.y += step.y;			
		}
		else 
		{
			t_max.z += t_delta.z;
			current_voxel.z += step.z;		
		}
	}
	return result;
}

bool ResolveRayModelIntersection(inout vec3 origin, vec3 direction, in VoxelModel model)
{
	AABB aabb = ModelAABB(model);
	bool result = PointInBox(origin, aabb.Min, aabb.Max); // Default is true in case origin is inside
	if (!result)
	{
		// Check if we are intersecting with grid
		float dist;
		result = RayBoxIntersection(origin, direction, aabb.Min, aabb.Max, dist);
		origin = origin + direction * (dist - EPSILON); // Move origin to first intersection
	}
	return result;
}

void StoreHitResult(in Ray ray, in RaymarchResult result, in VoxelModel model)
{
	ivec2 textureIndex = ivec2(gl_GlobalInvocationID.xy);	
	if (!result.OpaqueHit)
	{
		vec4 originalColor = imageLoad(o_Image, textureIndex);
		result.Color.rgb = mix(result.Color.rgb, originalColor.rgb, 1.0 - result.Color.a);
	}
	else // Store depth only for opaque hits
	{
		imageStore(o_DepthImage, textureIndex, vec4(result.Distance, 0,0,0)); // Store new depth
	}

	vec3 worldHitPosition = ray.Origin + (ray.Direction * result.Distance);
	vec3 worldNormal = mat3(model.InverseTransform) * -result.Normal;
	result.Color.rgb = CalculateDirLights(worldHitPosition, result.Color.rgb, worldNormal);

	imageStore(o_Image, textureIndex, result.Color); // Store color		
}


bool ValidPixel(ivec2 index)
{
	return index.x <= int(u_ViewportSize.x) && index.y <= int(u_ViewportSize.y);
}


layout(local_size_x = TILE_SIZE, local_size_y = TILE_SIZE, local_size_z = 1) in;
void main() 
{
	ivec2 textureIndex = ivec2(gl_GlobalInvocationID.xy);
	
	for (uint i = 0; i < NumModels; i++)
	{
		VoxelModel model = Models[i];
		Ray ray = CreateRay(u_CameraPosition.xyz, model.InverseTransform, textureIndex);
		vec3 origin = ray.Origin;
		vec3 direction = ray.Direction;
					
		// Check if ray intersects with model and move origin of ray
		if (!ResolveRayModelIntersection(origin, direction, model))
			continue;

		float currentDistance = imageLoad(o_DepthImage, textureIndex).r;
		RaymarchResult result;

		if (Models[i].TopGridIndex != -1 && u_Uniforms.UseTopGrid)
		{
			result = RaymarchTopGrid(ray, origin, direction, model, currentDistance, TopGrids[Models[i].TopGridIndex]);
		}
		else
		{
			result = RayMarch(ray, origin, direction, model, currentDistance, Models[i].MaxTraverses);		
		}

		if (result.Hit)		
			StoreHitResult(ray, result, model);	
	}
}