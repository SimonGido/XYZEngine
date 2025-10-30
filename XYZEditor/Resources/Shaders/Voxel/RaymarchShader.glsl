//#type compute
#version 460
#extension GL_EXT_shader_8bit_storage : enable


#include "Resources/Shaders/Includes/Math.glsl"
#include "Resources/Shaders/Includes/PBR.glsl"

#define TILE_SIZE 16
#define MAX_COLORS 1024
#define MAX_MODELS 1024
#define MULTI_COLOR 256
#define STACK_MAX 256
#define MODEL_GRID_MAX_CELLS 10 * 3 * 10
#define MAX_NODES 16384
#define MAX_DEPTH 5

const float EPSILON = 0.01;
const uint OPAQUE = 255;

layout(push_constant) uniform Uniforms
{
	bool UseOctree;
	bool UseModelGrid;
	bool UseBVH;
	bool ShowBVH;
	bool ShowDepth;
	bool ShowNormals;

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



struct VoxelModelBVHNode
{
	vec4 Min;
	vec4 Max;

	int Depth;
	int Data;
	int Left;
	int Right;
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
	uint  CellOffset;
	uint  CompressScale;
	bool  Compressed;
	float DistanceFromCamera;
	bool  Opaque;

	uint  Padding[1];
};

struct VoxelCompressedCell
{
	uint VoxelCount;
	uint VoxelOffset;
};

struct ModelGridCell
{
	uint ModelOffset;
	uint ModelCount;
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
};

layout(std430, binding = 19) readonly buffer buffer_Colors
{		
	uint ColorPallete[MAX_COLORS][256];
};

layout(std430, binding = 20) readonly buffer buffer_Compressed
{		
	VoxelCompressedCell CompressedCells[];
};

layout(std430, binding = 23) readonly buffer buffer_BVH
{		
	uint NodeCount;
	uint Padding[3];
	VoxelModelBVHNode Nodes[];
};

layout(std430, binding = 26) readonly buffer buffer_ModelGrid
{		
	ivec3	GridDimensions;
	vec3	GridCellSize;
	mat4	GridInverseTransform;
	ModelGridCell GridCells[MODEL_GRID_MAX_CELLS];
	uint	GridModelIndices[];
};

layout(std430, binding = 28) readonly buffer buffer_Octree
{		
	uint OctreeNodeCount;
	uint OctreePadding[3];
	VoxelModelOctreeNode OctreeNodes[MAX_NODES];
	uint OctreeModelIndices[MAX_MODELS];
};

layout(binding = 21, rgba32f) uniform image2D o_Image;
layout(binding = 22, r32f) uniform image2D o_DepthImage;
layout(binding = 24, rgba32f) uniform image2D o_Normal;
layout(binding = 25, rgba32f) uniform image2D o_Position;

// Helper functions

const float FarClip = 1000.0;
const float NearClip = 0.1;

float DistToDepth(float dist)
{
    return (dist - NearClip) / (FarClip - NearClip);
}
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
	color.a = bitfieldExtract(voxel, 24, 8) / 255.0;
	color.a = max(color.a, 0.0);
	color.rgb *= color.a;
	return color;
}


vec4 BlendColors(vec4 src, vec4 dst)
{
     return src + dst * (1.0 - src.a);
}

vec4 BlendColorsN(vec4 src, vec4 dst, int numSteps)
{
	float a = dst.a;
    float alphaFinal = 1.0 - pow(1.0 - a, float(numSteps));
    
    vec3 rgbFinal = dst.rgb * (alphaFinal / max(a, 0.0001)); 

    return BlendColors(src, vec4(rgbFinal, alphaFinal));
}

struct RaymarchResult
{
	vec4  Color;
	vec3  Normal;
	vec3  WorldHit;
	vec3  WorldNormal;
	float Distance;
	bool  Hit;
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
	ivec3 DecompressedVoxelOffset;
};


vec3 GetNormalFromState(in RaymarchState state, ivec3 step)
{
	if (state.Max.x < state.Max.y && state.Max.x < state.Max.z) 
	{
		return vec3(float(-step.x), 0.0, 0.0);
	}
	else if (state.Max.y < state.Max.z) 
	{
		return vec3(0.0, float(step.y), 0.0);
	}
	else 
	{
		return vec3(0.0, 0.0, float(-step.z));
	}	
}

int CalculateNumberOfSteps(in Ray ray, float tMin, float tMax, float voxelSize)
{
	vec3 rayStart		= ray.Origin + ray.Direction * (tMin - EPSILON);
	vec3 rayEnd			= ray.Origin + ray.Direction * (tMax - EPSILON);
	ivec3 startVoxel	= ivec3(floor(rayStart / voxelSize));
	ivec3 endVoxel		= ivec3(floor(rayEnd / voxelSize));

	ivec3 diff = abs(endVoxel - startVoxel);
	return 1 + diff.x + diff.y + diff.z;
}

float GetNextDistance(in RaymarchState state, ivec3 step, vec3 delta)
{
	if (state.Max.x < state.Max.y && state.Max.x < state.Max.z)
	{
		return state.Max.x;
	}
	else if (state.Max.y < state.Max.z)
	{
		return state.Max.y;

	}
	else
	{
		return state.Max.z;
	}
}

void PerformStep(inout RaymarchState state, ivec3 step, vec3 delta)
{
	if (state.Max.x < state.Max.y && state.Max.x < state.Max.z) 
	{
		state.Distance = state.Max.x;
		state.Normal = vec3(float(-step.x), 0.0, 0.0);
		state.Max.x += delta.x;
		state.CurrentVoxel.x += step.x;
		state.MaxSteps.x--;
	}
	else if (state.Max.y < state.Max.z) 
	{
		state.Distance = state.Max.y;
		state.Normal = vec3(0.0, float(step.y), 0.0);
		state.Max.y += delta.y;
		state.CurrentVoxel.y += step.y;			
		state.MaxSteps.y--;
	}
	else 
	{
		state.Distance = state.Max.z;
		state.Normal = vec3(0.0, 0.0, float(-step.z));
		state.Max.z += delta.z;
		state.CurrentVoxel.z += step.z;		
		state.MaxSteps.z--;
	}	
}

RaymarchState CreateRaymarchState(in Ray ray, float tMin, ivec3 step, ivec3 maxSteps, vec3 voxelSize, ivec3 decompressedVoxelOffset)
{
	RaymarchState state;
	vec3 rayStart					= ray.Origin + ray.Direction * (tMin - EPSILON);
	ivec3 decompressedCurrentVoxel	= ivec3(floor(rayStart / voxelSize));
	state.Distance					= tMin;
	state.CurrentVoxel				= decompressedCurrentVoxel - decompressedVoxelOffset;
	state.MaxSteps					= maxSteps;
	state.DecompressedVoxelOffset	= decompressedVoxelOffset;
	
	vec3 next_boundary = vec3(
		float((step.x > 0) ? decompressedCurrentVoxel.x + 1 : decompressedCurrentVoxel.x) * voxelSize.x,
		float((step.y > 0) ? decompressedCurrentVoxel.y + 1 : decompressedCurrentVoxel.y) * voxelSize.y,
		float((step.z > 0) ? decompressedCurrentVoxel.z + 1 : decompressedCurrentVoxel.z) * voxelSize.z
	);

	state.Max = tMin + (next_boundary - rayStart) / ray.Direction; // we will move along the axis with the smallest value
	state.Normal = GetNormalFromState(state, step);
	return state;
}

RaymarchState CreateRaymarchState(in Ray ray, float tMin, ivec3 step, ivec3 maxSteps, float voxelSize, ivec3 decompressedVoxelOffset)
{
	return CreateRaymarchState(ray, tMin, step, maxSteps, vec3(voxelSize, voxelSize, voxelSize), decompressedVoxelOffset);
}

RaymarchResult RayMarchModel(
	in Ray			ray,
	float			tMin, 
	in VoxelModel	model, 
	vec4			currentColor, 
	float			currentDistance, 
	ivec3			decompressedVoxelOffset, 
	vec4			startColor,
	float			decompressedVoxelSize
)
{
	RaymarchResult result;
	result.Color	= startColor;
	result.Hit		= false;
	result.Distance	= 0.0;

	ivec3 step = ivec3(
		(ray.Direction.x > 0.0) ? 1 : -1,
		(ray.Direction.y > 0.0) ? 1 : -1,
		(ray.Direction.z > 0.0) ? 1 : -1
	);
	uint width				= model.Width;
	uint height				= model.Height;
	uint depth				= model.Depth;
	uint voxelOffset		= model.VoxelOffset;
	uint colorPalleteIndex	= model.ColorIndex;
	float voxelSize			= model.VoxelSize;

	vec3 delta				= voxelSize / ray.Direction * vec3(step);	
	ivec3 maxSteps			= ivec3(width, height, depth);
	RaymarchState state		= CreateRaymarchState(ray, tMin, step, maxSteps, voxelSize, decompressedVoxelOffset);

	while (state.MaxSteps.x >= 0 && state.MaxSteps.y >= 0 && state.MaxSteps.z >= 0) 
	{					
		if (state.Distance > currentDistance)
		{	
			if (currentColor.a >= 1.0) // Something opaque in front of us
				break;
		}
		if (IsValidVoxel(state.CurrentVoxel, width, height, depth))
		{
			uint voxelIndex = Index3D(state.CurrentVoxel, width, height) + voxelOffset;
			uint colorIndex = uint(Voxels[voxelIndex]);
			uint voxel		= ColorPallete[colorPalleteIndex][colorIndex];
			if (voxel != 0)
			{
				if (result.Hit == false) // First hit
				{
					result.Distance = state.Distance;
					result.Normal	= state.Normal;
					result.Hit		= true;
				}
				
				float tMin = state.Distance;
				float tMax = GetNextDistance(state, step, delta);
				tMax = min(tMax, currentDistance);
				
				int   numSteps	 = CalculateNumberOfSteps(ray, tMin, tMax, decompressedVoxelSize);
				vec4  voxelColor = VoxelToColor(voxel);
				result.Color = BlendColorsN(result.Color, voxelColor, numSteps);	
				if (result.Color.a >= 1.0)
					return result;	
			}
		}
		PerformStep(state, step, delta); // Hit was not opaque we continue raymarching, perform step to get out of transparent voxel	
	}	
	return result;
}

RaymarchResult RaymarchCompressed(in Ray ray, float tMin, in VoxelModel model, vec4 currentColor, float currentDistance)
{
	RaymarchResult result;
	result.Hit		= false;
	result.Color	= vec4(0,0,0,0);
	result.Distance = 0.0;
	
	ivec3 step = ivec3(
		(ray.Direction.x > 0.0) ? 1 : -1,
		(ray.Direction.y > 0.0) ? 1 : -1,
		(ray.Direction.z > 0.0) ? 1 : -1
	);
	ivec3 maxSteps		= ivec3(model.Width, model.Height, model.Depth);
	vec3 t_delta		= model.VoxelSize / ray.Direction * vec3(step);
	RaymarchState state = CreateRaymarchState(ray, tMin, step, maxSteps, model.VoxelSize, ivec3(0,0,0));

	while (state.MaxSteps.x >= 0 && state.MaxSteps.y >= 0 && state.MaxSteps.z >= 0)
	{		
		if (state.Distance > currentDistance)
		{
			if (currentColor.a >= 1.0)
				break;
		}
		if (IsValidVoxel(state.CurrentVoxel, model.Width, model.Height, model.Depth))
		{		
			uint cellIndex					= Index3D(state.CurrentVoxel, model.Width, model.Height) + model.CellOffset;
			VoxelCompressedCell cell		= CompressedCells[cellIndex];
				
			// Calculates real coordinates of CurrentVoxel in decompressed model
			VoxelModel cellModel;
			int dimSize = int(pow(cell.VoxelCount, 1.0 / 3.0));
			float baseVoxelSize = model.VoxelSize / max(model.CompressScale, 1);
			ivec3 decompressedVoxelOffset = state.CurrentVoxel * dimSize;
			
			cellModel.ColorIndex	= model.ColorIndex;
			cellModel.VoxelOffset	= model.VoxelOffset + cell.VoxelOffset;		
			cellModel.Width			= dimSize;
			cellModel.Height		= dimSize;
			cellModel.Depth			= dimSize;
			cellModel.VoxelSize		= model.VoxelSize / dimSize;
			
			RaymarchResult newResult = RayMarchModel(
				ray, 
				state.Distance - EPSILON, 
				cellModel, 
				currentColor, 
				currentDistance, 
				decompressedVoxelOffset, 
				result.Color,
				baseVoxelSize
			);				
			if (newResult.Hit)
			{		
				if (result.Hit == false)
				{
					result.Normal = newResult.Normal;
					result.Distance = newResult.Distance;
					result.Hit = true;
				}
				result.Color = newResult.Color;
				if (result.Color.a >= 1.0)
					return result;	
			}
		}
		PerformStep(state, step, t_delta);
	}
	return result;
}

bool RayBoxIntersectionInside(in vec3 origin, vec3 direction, in vec3 aabbMin, in vec3 aabbMax, out float tMin, out float tMax)
{
	tMin = 0.0;
	bool result = PointInBox(origin, aabbMin, aabbMax);
	if (!result)
		result = RayBoxIntersection(origin, direction, aabbMin, aabbMax, tMin, tMax);
	return result;
}

bool ResolveRayModelIntersection(in vec3 origin, vec3 direction, in VoxelModel model, out float tMin, out float tMax)
{
	AABB aabb = ModelAABB(model);
	return RayBoxIntersectionInside(origin, direction, aabb.Min, aabb.Max, tMin, tMax);
}


void StoreHitResult(in RaymarchResult result, float currentDistance)
{
	ivec2 textureIndex = ivec2(gl_GlobalInvocationID.xy);	
	// Store smaller depth
	if (currentDistance >= result.Distance)
	{
		imageStore(o_Normal, textureIndex, vec4(result.WorldNormal, 1.0));
		imageStore(o_Position, textureIndex, vec4(result.WorldHit, 1.0));
		imageStore(o_DepthImage, textureIndex, vec4(result.Distance, 0,0,0)); // Store new depth
	}
	if (u_Uniforms.ShowDepth)
	{
		float depth = DistToDepth(imageLoad(o_DepthImage, textureIndex).r);
		imageStore(o_Image, textureIndex, vec4(depth, depth, depth, depth)); // Store color	
	}
	else if (u_Uniforms.ShowNormals)
	{
		vec4 normal = imageLoad(o_Normal, textureIndex);
		imageStore(o_Image, textureIndex, normal); // Store color	
	}
}

bool DrawModel(in Ray cameraRay, in VoxelModel model, vec4 currentColor, float currentDistance, inout vec4 drawColor, inout float drawDistance)
{
	ivec2 textureIndex = ivec2(gl_GlobalInvocationID.xy);
	
	Ray modelRay = CreateRay(u_CameraPosition.xyz, model.InverseTransform, textureIndex);

	float tMin = 0.0;
	float tMax = 0.0;

	// Check if ray intersects with model and move origin of ray
	if (!ResolveRayModelIntersection(modelRay.Origin, modelRay.Direction, model, tMin, tMax))
		return false;

	RaymarchResult result;
	if (model.Compressed)
	{
		result = RaymarchCompressed(modelRay, tMin, model, currentColor, currentDistance);
	}
	else
	{
		result = RayMarchModel(modelRay, tMin, model, currentColor, currentDistance, ivec3(0,0,0), vec4(0,0,0,0), model.VoxelSize);	
	}
	if (result.Hit)		
	{ 
		result.WorldHit = cameraRay.Origin + (cameraRay.Direction * result.Distance);
		result.WorldNormal = mat3(model.InverseTransform) * result.Normal;

		StoreHitResult(result, currentDistance);	
		drawDistance = result.Distance;
		drawColor = result.Color; // Our color becomes new last drawColor
		return true;
	}
	return false;
}


void RaycastBVH(in Ray cameraRay)
{
	if (NodeCount == 0)
		return;

	VoxelModelBVHNode root = Nodes[0];

	float tMin = 0.0;
	float tMax = 0.0;
	if (!RayBoxIntersectionInside(cameraRay.Origin, cameraRay.Direction, root.Min.xyz, root.Max.xyz, tMin, tMax))
		return;

	int stackIndex = 0;
	uint stack[STACK_MAX];
	stack[stackIndex++] = 0; // Start with the root node

	ivec2 textureIndex = ivec2(gl_GlobalInvocationID.xy);

	bool modelDrawn[MAX_MODELS];
	for (int i = 0; i < NumModels; i++)
		modelDrawn[i] = false;

	int hitCount = 0;
	int maxHitIndex = -1;
	while (stackIndex > 0)
	{
		stackIndex--;
		uint nodeIndex = stack[stackIndex];
		VoxelModelBVHNode node = Nodes[nodeIndex];

		if (node.Data != -1)
		{
			maxHitIndex = max(maxHitIndex, node.Data);
			modelDrawn[node.Data] = true;
		}
		else if (RayAABBOverlap(cameraRay.Origin, cameraRay.Direction, node.Min.xyz, node.Max.xyz))
		{		
			if (node.Left != -1)
			{	
				stack[stackIndex++] = node.Left;
				stack[stackIndex++] = node.Right;
			}
			hitCount++;
		}
	}

	float currentDistance = FLT_MAX;
	vec4  currentColor	  = vec4(0,0,0,0);

	float newDistance	  = 0.0;
	vec4  newColor		  = vec4(0,0,0,0);
	for (int i = 0; i <= maxHitIndex; i++)
	{
		if (modelDrawn[i])
		{
			float drawDistance = FLT_MAX;
			VoxelModel model = Models[i];
			if (DrawModel(cameraRay, model, currentColor, currentDistance, newColor, newDistance))
			{
				if (newDistance < currentDistance)
				{
					currentColor = BlendColors(newColor, currentColor);
					currentDistance = newDistance;
				}
				else
				{
					currentColor = BlendColors(currentColor, newColor);
				}
			}	
		}
	}

	if (!u_Uniforms.ShowDepth && !u_Uniforms.ShowNormals)
		imageStore(o_Image, textureIndex, currentColor);

	if (u_Uniforms.ShowBVH)
	{
		vec3 gradient = GetGradient(hitCount / 5) * 0.3;
		vec4 origColor = imageLoad(o_Image, textureIndex);
		origColor.rgb += gradient;
		imageStore(o_Image, textureIndex, vec4(gradient.rgb, origColor.a));		
	}
}

void RaycastOctree(in Ray ray)
{
	ivec2 textureIndex = ivec2(gl_GlobalInvocationID.xy);

	bool modelDrawn[MAX_MODELS];
	for (int i = 0; i < NumModels; i++)
		modelDrawn[i] = false;

	int maxModelHit = -1;
	int stack[MAX_DEPTH * 5];
	int stackIndex = 0;
	stack[stackIndex++] = 0; // Start with the root node
	int hitCount = 0;
	while (stackIndex > 0)
	{
		stackIndex--;
		int nodeIndex = stack[stackIndex];
		VoxelModelOctreeNode node = OctreeNodes[nodeIndex];
			
		for (int i = node.DataStart; i < node.DataEnd; i++)
		{
			float drawDistance;
			uint modelIndex = OctreeModelIndices[i];
			modelDrawn[modelIndex] = true;
			maxModelHit = max(maxModelHit, int(modelIndex));
		}

		if (!node.IsLeaf)
		{
			for (int c = 0; c < 8; c++)
			{
				VoxelModelOctreeNode child = OctreeNodes[node.Children[c]];
				if (child.DataEnd - child.DataStart == 0 && child.IsLeaf)
					continue;

				float tMin = 0.0;
				float tMax = 0.0;
				if (RayBoxIntersectionInside(ray.Origin, ray.Direction, child.Min.xyz, child.Max.xyz, tMin, tMax))
				{
					stack[stackIndex++] = node.Children[c];
					hitCount++;
				}
			}
		}
	}

	float currentDistance = FLT_MAX;
	vec4  currentColor	  = vec4(0,0,0,0);

	float newDistance	  = 0.0;
	vec4  newColor		  = vec4(0,0,0,0);

	for (int i = 0; i <= maxModelHit; i++)
	{
		if (modelDrawn[i])
		{
			VoxelModel model = Models[i];
			if (DrawModel(ray, model, currentColor, currentDistance, newColor, newDistance))
			{
				if (newDistance < currentDistance)
				{
					currentColor = BlendColors(newColor, currentColor);
					currentDistance = newDistance;
				}
				else
				{
					currentColor = BlendColors(currentColor, newColor);
				}
			}	
		}
	}

	if (!u_Uniforms.ShowDepth && !u_Uniforms.ShowNormals)
		imageStore(o_Image, textureIndex, currentColor);

	if (u_Uniforms.ShowBVH)
	{
		vec3 gradient = GetGradient(hitCount) * 0.3;
		vec4 origColor = imageLoad(o_Image, textureIndex);
		origColor.rgb += gradient;
		imageStore(o_Image, textureIndex, vec4(gradient.rgb, origColor.a));		
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

	Ray cameraRay = CreateRay(u_CameraPosition.xyz, textureIndex);

	if (u_Uniforms.UseBVH)
	{
		RaycastBVH(cameraRay);
	}
	else if (u_Uniforms.UseOctree)
	{
		RaycastOctree(cameraRay);
	}
	else
	{		
		float currentDistance = FLT_MAX;
		vec4  currentColor	  = vec4(0,0,0,0);

		float newDistance	  = 0.0;
		vec4  newColor		  = vec4(0,0,0,0);
		
		for (uint i = 0; i < NumModels; i++)
		{
			VoxelModel model = Models[i];
			if (currentColor.a >= 1.0 && model.DistanceFromCamera > currentDistance)
				continue;
		
			if (DrawModel(cameraRay, model, currentColor, currentDistance, newColor, newDistance))
			{
				if (newDistance < currentDistance)
				{
					currentColor = BlendColors(newColor, currentColor);
					currentDistance = newDistance;
				}
				else
				{
					currentColor = BlendColors(currentColor, newColor);
				}
			}	
		}
		if (!u_Uniforms.ShowDepth && !u_Uniforms.ShowNormals)
			imageStore(o_Image, textureIndex, currentColor);
	}
}