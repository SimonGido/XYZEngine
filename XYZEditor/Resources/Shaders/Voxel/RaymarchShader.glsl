//#type compute
#version 460
#extension GL_EXT_shader_8bit_storage : enable

#include "Resources/Shaders/Includes/Math.glsl"
#include "Resources/Shaders/Includes/PBR.glsl"

#define TILE_SIZE 16
#define MAX_COLORS 1024
#define MAX_MODELS 1024
#define MULTI_COLOR 256
#define MAX_DEPTH 16
#define MAX_NODES 16384


const float EPSILON = 0.01;
const uint OPAQUE = 255;

layout(push_constant) uniform Uniforms
{
	bool UseAccelerationGrid;
	bool UseOctree;
	bool ShowOctree;
	bool ShowAABB;

} u_Uniforms;

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


struct VoxelModelOctreeNode
{
	vec4 Min;
	vec4 Max;

	int  Children[8];
	
	bool IsLeaf;
	int  DataStart;
	int  DataEnd;

	uint Padding;
};

struct VoxelAccelerationGrid
{
	uint  CellsOffset;

	uint  Width;
	uint  Height;
	uint  Depth;
	float Size;

	uint Padding[3];
};

struct VoxelModel
{
	mat4  InverseTransform;

	uint  VoxelOffset;
	uint  Width;
	uint  Height;
	uint  Depth;
	uint  ColorIndex;

	float VoxelSize;
	int   AccelerationGridIndex;

	uint  Padding;
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
	VoxelAccelerationGrid AccelerationGrids[];
};

layout(std430, binding = 19) readonly buffer buffer_Colors
{		
	uint ColorPallete[MAX_COLORS][256];
};

layout(std430, binding = 20) readonly buffer buffer_AccelerationGrids
{		
	uint8_t AccelerationGridCells[];
};

layout(std430, binding = 23) readonly buffer buffer_Octree
{		
	uint NodeCount;
	uint Padding[3];
	VoxelModelOctreeNode Nodes[MAX_NODES];
	uint ModelIndices[MAX_MODELS];
};

layout(binding = 21, rgba16f) uniform image2D o_Image;
layout(binding = 22, r32f) uniform image2D o_DepthImage;



Ray g_CameraRay;

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
	ray.Direction = normalize(ray.Direction);

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
	ray.Direction = normalize(ray.Direction);

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
	vec3  WorldHit;
	vec3  WorldNormal;
	vec3  Throughput;
	ivec3 FinalVoxel;
	ivec3 NextVoxel;
	float Distance;
	bool  Hit;
	uint  ColorUINT;
};


struct RaymarchState
{
	vec4  Color;
	vec3  Max;
	vec3  Normal;
	ivec3 CurrentVoxel;
	ivec3 MaxSteps;
	bool  Hit;
	float Distance;
	uint  ColorUINT;
};


void PerformStep(inout RaymarchState state, ivec3 step, vec3 delta)
{
	if (state.Max.x < state.Max.y && state.Max.x < state.Max.z) 
	{
		state.Normal = vec3(float(-step.x), 0.0, 0.0);
		state.Max.x += delta.x;
		state.CurrentVoxel.x += step.x;
		state.MaxSteps.x--;
	}
	else if (state.Max.y < state.Max.z) 
	{
		state.Normal = vec3(0.0, float(-step.y), 0.0);
		state.Max.y += delta.y;
		state.CurrentVoxel.y += step.y;			
		state.MaxSteps.y--;
	}
	else 
	{
		state.Normal = vec3(0.0, 0.0, float(-step.z));
		state.Max.z += delta.z;
		state.CurrentVoxel.z += step.z;		
		state.MaxSteps.z--;
	}	
}



void RayMarch(in Ray ray, inout RaymarchState state, vec3 delta, ivec3 step, in VoxelModel model, float currentDistance)
{
	state.Hit = false;
	state.Color = vec4(0,0,0,0);

	uint width = model.Width;
	uint height = model.Height;
	uint depth = model.Depth;
	uint voxelOffset = model.VoxelOffset;
	uint colorPalleteIndex = model.ColorIndex;
	float voxelSize = model.VoxelSize;

	while (state.MaxSteps.x >= 0 && state.MaxSteps.y >= 0 && state.MaxSteps.z >= 0)
	{
		state.Distance = VoxelDistanceFromRay(ray.Origin, ray.Direction, state.CurrentVoxel, model.VoxelSize);
		if (state.Distance > currentDistance)
			break;

		if (IsValidVoxel(state.CurrentVoxel, width, height, depth))
		{
			uint voxelIndex = Index3D(state.CurrentVoxel, width, height) + voxelOffset;
			uint colorIndex = uint(Voxels[voxelIndex]);
			uint voxel = ColorPallete[colorPalleteIndex][colorIndex];
			state.ColorUINT = voxel;
			if (voxel != 0)
			{
				state.Color = VoxelToColor(voxel);
				state.Hit = true;				
				break;
			}
		}
		PerformStep(state, step, delta);
	}
}



RaymarchState CreateRaymarchState(in Ray ray, vec3 origin, vec3 direction, ivec3 step, ivec3 maxSteps, float voxelSize)
{
	RaymarchState state;
	state.Distance = distance(ray.Origin, origin);
	state.CurrentVoxel = ivec3(floor(origin / voxelSize));
	state.MaxSteps = maxSteps;
	state.ColorUINT = 0;
	vec3 next_boundary = vec3(
		float((step.x > 0) ? state.CurrentVoxel.x + 1 : state.CurrentVoxel.x) * voxelSize,
		float((step.y > 0) ? state.CurrentVoxel.y + 1 : state.CurrentVoxel.y) * voxelSize,
		float((step.z > 0) ? state.CurrentVoxel.z + 1 : state.CurrentVoxel.z) * voxelSize
	);

	state.Max = (next_boundary - origin) / direction; // we will move along the axis with the smallest value
	return state;
}

const ivec3 neighbours[6] = {
	ivec3(-1, 0, 0),
	ivec3( 1, 0, 0),
	ivec3( 0, 1, 0),
	ivec3( 0,-1, 0),

	ivec3(0, 0, 1),
	ivec3(0, 0, -1)
};

bool IdenticalNeighbour(ivec3 voxel, in VoxelModel model)
{
	uint current = Index3D(voxel, model.Width, model.Height) + model.VoxelOffset;
	uint colorIndex = uint(Voxels[current]);
	for (int i = 0; i < 6; i++)
	{
		if (IsValidVoxel(voxel + neighbours[i], model.Width, model.Height, model.Depth))
		{
			uint neighbour = Index3D(voxel + neighbours[i], model.Width, model.Height) + model.VoxelOffset;
			uint neighbourColorIndex = uint(Voxels[neighbour]);
			if (neighbourColorIndex == colorIndex)
				return true;
		}
	}
	return false;
}

RaymarchResult RayMarchSteps(in Ray ray, vec4 startColor, uint startColorUINT, vec3 throughput, vec3 origin, vec3 direction, in VoxelModel model, float currentDistance, ivec3 maxSteps)
{
	RaymarchResult result;
	result.Color = startColor;
	result.Hit = false;
	result.Distance = 0;
	result.Throughput = throughput;
	result.ColorUINT = startColorUINT;

	ivec3 step = ivec3(
		(direction.x > 0.0) ? 1 : -1,
		(direction.y > 0.0) ? 1 : -1,
		(direction.z > 0.0) ? 1 : -1
	);
			
	float voxelSize = model.VoxelSize;
	vec3 delta = voxelSize / direction * vec3(step);	

	RaymarchState state = CreateRaymarchState(ray, origin, direction, step, maxSteps, voxelSize);
	// Continue raymarching until we hit opaque object or we are out of traverses
	while (state.MaxSteps.x >= 0 && state.MaxSteps.y >= 0 && state.MaxSteps.z >= 0)
	{		
		RayMarch(ray, state, delta, step, model, currentDistance);
			
		// We passed depth test
		if (state.Hit) // We hit something so mix colors together
		{
			if (result.ColorUINT != state.ColorUINT)
			{
				result.Color.rgb += state.Color.rgb * result.Throughput;
				result.Hit = true;
				result.Normal = state.Normal;
				result.FinalVoxel = state.CurrentVoxel;	
				result.ColorUINT = state.ColorUINT;
				result.Throughput *= mix(vec3(1.0), state.Color.rgb, state.Color.a) * (1.0 - state.Color.a);
			}
		}
		

		// Test if we can pass through
		if (result.Throughput.x <= EPSILON && result.Throughput.y <= EPSILON && result.Throughput.z <= EPSILON)
			break;	

		PerformStep(state, step, delta); // Hit was not opaque we continue raymarching, perform step to get out of transparent voxel
		
		if (state.Distance > currentDistance) // if new depth is bigger than currentDepth it means there is something in front of us
			break;		
		
	}
	result.Color.a = 1.0 - CalculateLuminance(result.Throughput);
	result.Distance = state.Distance;
	result.NextVoxel = state.CurrentVoxel;
	return result;
}


RaymarchResult RayMarch(in Ray ray, vec4 startColor, uint startColorUINT, vec3 throughput, vec3 origin, vec3 direction, in VoxelModel model, float currentDistance)
{
	return RayMarchSteps(ray, startColor, startColorUINT, throughput, origin, direction, model, currentDistance, ivec3(model.Width, model.Height, model.Depth));
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

// TODO: refactor and fix it
RaymarchResult RaymarchAccelerationGrid(in Ray ray, vec3 origin, vec3 direction, in VoxelModel model, float currentDistance, in VoxelAccelerationGrid grid)
{
	RaymarchResult result;
	result.ColorUINT = 0;
	result.Hit = false;

	ivec2 textureIndex = ivec2(gl_GlobalInvocationID.xy);
	ivec3 current_voxel = ivec3(floor(origin / grid.Size));

	ivec3 step = ivec3(
		(direction.x > 0.0) ? 1 : -1,
		(direction.y > 0.0) ? 1 : -1,
		(direction.z > 0.0) ? 1 : -1
	);
	vec3 next_boundary = vec3(
		float((step.x > 0) ? current_voxel.x + 1 : current_voxel.x) * grid.Size,
		float((step.y > 0) ? current_voxel.y + 1 : current_voxel.y) * grid.Size,
		float((step.z > 0) ? current_voxel.z + 1 : current_voxel.z) * grid.Size
	);
	

	vec3 t_max = (next_boundary - origin) / direction; // we will move along the axis with the smallest value
	vec3 t_delta = grid.Size / direction * vec3(step);	

	vec3 newOrigin = origin;
	vec4 color = vec4(0,0,0,0);
	vec3 throughput = vec3(1,1,1);
	uint scale = uint(ceil(grid.Size / model.VoxelSize));
	ivec3 maxSteps = ivec3(grid.Width, grid.Height, grid.Depth);

	while (maxSteps.x >= 0 && maxSteps.y >= 0 && maxSteps.z >= 0)
	{
		float newDistance = VoxelDistanceFromRay(ray.Origin, ray.Direction, current_voxel, grid.Size);
		if (newDistance > currentDistance)
			break;


		if (IsValidVoxel(current_voxel, grid.Width, grid.Height, grid.Depth))
		{
			uint voxelIndex = Index3D(current_voxel, grid.Width, grid.Height) + grid.CellsOffset;
			if (uint(AccelerationGridCells[voxelIndex]) != 0)
			{			
				float dist = VoxelDistanceFromRay(newOrigin, direction, current_voxel, grid.Size);
				newOrigin += direction * (dist - EPSILON); // Move origin to hit of top grid cell

				// Raymarch from new origin						
				RaymarchResult newResult = RayMarchSteps(ray, color, result.ColorUINT, throughput, newOrigin, direction, model, currentDistance, ivec3(scale, scale, scale));
				if (newResult.Hit)
				{
					color = newResult.Color;
					throughput = newResult.Throughput;
					result = newResult;
				}
				if (throughput.x <= EPSILON && throughput.y <= EPSILON && throughput.z <= EPSILON)
					return result;				
			}
		}

		if (t_max.x < t_max.y && t_max.x < t_max.z) 
		{
			t_max.x += t_delta.x;
			current_voxel.x += step.x;
			maxSteps.x--;
		}
		else if (t_max.y < t_max.z) 
		{
			t_max.y += t_delta.y;
			current_voxel.y += step.y;		
			maxSteps.y--;
		}
		else 
		{
			t_max.z += t_delta.z;
			current_voxel.z += step.z;	
			maxSteps.z--;
		}
	}
	result.Color = color;
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

void StoreHitResult(in RaymarchResult result)
{
	ivec2 textureIndex = ivec2(gl_GlobalInvocationID.xy);	

	bool opaque = result.Throughput.x <= EPSILON && result.Throughput.y <= EPSILON && result.Throughput.z <= EPSILON;

	if (!opaque)
	{
		vec4 originalColor = imageLoad(o_Image, textureIndex);
		result.Color.rgb = mix(result.Color.rgb, originalColor.rgb, 1.0 - result.Color.a);
	}
	else // Store depth only for opaque hits
	{
		imageStore(o_DepthImage, textureIndex, vec4(result.Distance, 0,0,0)); // Store new depth
	}

	if (opaque)
		result.Color.rgb = CalculateDirLights(result.WorldHit, result.Color.rgb, result.WorldNormal);

	result.Color.a = 1.0;
	imageStore(o_Image, textureIndex, result.Color); // Store color		
}

bool DrawModel(uint modelIndex)
{
	vec4 startColor = vec4(0, 0, 0, 0);
	vec3 startThroughput = vec3(1, 1, 1);
	ivec2 textureIndex = ivec2(gl_GlobalInvocationID.xy);
	
	VoxelModel model = Models[modelIndex];
	Ray ray = CreateRay(u_CameraPosition.xyz, model.InverseTransform, textureIndex);
	vec3 origin = ray.Origin;
	vec3 direction = ray.Direction;
				
	float currentDistance = imageLoad(o_DepthImage, textureIndex).r;
	// Check if ray intersects with model and move origin of ray
	if (!ResolveRayModelIntersection(origin, direction, model))
		return false;

	RaymarchResult result;
	if (model.AccelerationGridIndex != -1 && u_Uniforms.UseAccelerationGrid)
	{
		result = RaymarchAccelerationGrid(ray, origin, direction, model, currentDistance, AccelerationGrids[model.AccelerationGridIndex]);
	}
	else
	{
		result = RayMarch(ray, startColor, 0, startThroughput, origin, direction, model, currentDistance);	
	}
	if (result.Hit)		
	{ 
		result.WorldHit = g_CameraRay.Origin + (g_CameraRay.Direction * result.Distance);
		result.WorldNormal = mat3(model.InverseTransform) * -result.Normal;
		StoreHitResult(result);	
		return result.Color.a == 1.0; // We hit opaque object
	}
	return false;
}



void RaycastOctree(in Ray ray)
{
	ivec2 textureIndex = ivec2(gl_GlobalInvocationID.xy);
	float currentDistance = imageLoad(o_DepthImage, textureIndex).r;
	int stack[MAX_DEPTH * 5];
	int stackIndex = 0;
	stack[stackIndex++] = 0; // Start with the root node
	

	while (stackIndex > 0)
	{
		stackIndex--;
		int nodeIndex = stack[stackIndex];
		VoxelModelOctreeNode node = Nodes[nodeIndex];
			
		for (int i = node.DataStart; i < node.DataEnd; i++)
		{
			DrawModel(ModelIndices[i]);
		}
		float currentDistance = imageLoad(o_DepthImage, textureIndex).r;
		if (!node.IsLeaf)
		{
			for (int c = 0; c < 8; c++)
			{
				VoxelModelOctreeNode child = Nodes[node.Children[c]];
				if (child.DataEnd - child.DataStart == 0 && child.IsLeaf)
					continue;

				float dist = -FLT_MAX;
				if (RayBoxIntersection(ray.Origin, ray.Direction, child.Min.xyz, child.Max.xyz, dist))
				{
					if (dist < currentDistance)
						stack[stackIndex++] = node.Children[c];
				}
			}
		}
	}
	// Debug stuff
	if (u_Uniforms.ShowOctree)
	{
		stack[stackIndex++] = 0; // Start with the root node
		while (stackIndex > 0)
		{
			stackIndex--;
			int nodeIndex = stack[stackIndex];
			VoxelModelOctreeNode node = Nodes[nodeIndex];

			float numModels = float(node.DataEnd - node.DataStart);
			vec3 gradient = GetGradient(numModels) * 0.1;
			vec4 origColor = imageLoad(o_Image, textureIndex);
			origColor.rgb += gradient;
			imageStore(o_Image, textureIndex, origColor); // Store color

			if (!node.IsLeaf)
			{
				for (int c = 0; c < 8; c++)
				{
					VoxelModelOctreeNode child = Nodes[node.Children[c]];
					if (child.DataEnd - child.DataStart == 0 && child.IsLeaf)
						continue;
					
					if (RayAABBOverlap(ray.Origin, ray.Direction, child.Min.xyz, child.Max.xyz))
					{
						stack[stackIndex++] = node.Children[c];
					}
				}
			}
		}	
	}
	if (u_Uniforms.ShowAABB)
	{
		stack[stackIndex++] = 0; // Start with the root node
		vec4 origColor = imageLoad(o_Image, textureIndex);
		while (stackIndex > 0)
		{
			stackIndex--;
			int nodeIndex = stack[stackIndex];
			VoxelModelOctreeNode node = Nodes[nodeIndex];
			for (int i = node.DataStart; i < node.DataEnd; i++)
			{
				VoxelModel model = Models[ModelIndices[i]];
				AABB aabb = ModelAABB(model);
				TransformAABB(inverse(model.InverseTransform), aabb.Min, aabb.Max);
				if (RayAABBOverlap(ray.Origin, ray.Direction, aabb.Min, aabb.Max))
				{			
					origColor.rgb += vec3(0.5, 0.4, 0.3);				
				}
			}			
			if (!node.IsLeaf)
			{
				for (int c = 0; c < 8; c++)
				{
					VoxelModelOctreeNode child = Nodes[node.Children[c]];
					if (child.DataEnd - child.DataStart == 0 && child.IsLeaf)
						continue;

					if (RayAABBOverlap(ray.Origin, ray.Direction, child.Min.xyz, child.Max.xyz))
					{
						stack[stackIndex++] = node.Children[c];
					}
				}
			}
		}	
		imageStore(o_Image, textureIndex, origColor); // Store color
	}
}



bool ValidPixel(ivec2 index)
{
	return index.x <= int(u_ViewportSize.x) && index.y <= int(u_ViewportSize.y);
}


layout(local_size_x = TILE_SIZE, local_size_y = TILE_SIZE, local_size_z = 1) in;
void main() 
{	
	ivec2 textureIndex = ivec2(gl_GlobalInvocationID.xy);
	if (!ValidPixel(textureIndex))
		return;

	g_CameraRay = CreateRay(u_CameraPosition.xyz, textureIndex);

	if (u_Uniforms.UseOctree)
	{
		RaycastOctree(g_CameraRay);
	}
	else
	{
		for (uint i = 0; i < NumModels; i++)
		{
			DrawModel(i);
		}
	}
}