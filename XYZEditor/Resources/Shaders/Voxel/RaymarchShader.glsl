//#type compute
#version 460
#extension GL_EXT_shader_8bit_storage : enable

#include "Resources/Shaders/Includes/Math.glsl"
#include "Resources/Shaders/Includes/PBR.glsl"

#define TILE_SIZE 16
#define MAX_COLORS 1024
#define MAX_MODELS 1024
#define MAX_NODES 4096
#define MAX_DEPTH 16

const float EPSILON = 0.01;
const uint OPAQUE = 255;
const int MULTI_COLOR = 256;

layout(push_constant) uniform Uniforms
{
	bool UseOctree; 
	bool ShowOctree;
	bool ShowAABB;
	bool ShowPixelComplexity;
	bool OnlyFilledNodes;

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


struct VoxelOctreeNode
{
	uint	Size;
	uint	X, Y, Z;

	uint	Children[8];

	uint	ColorIndex;
	uint	IsLeaf;

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

	float VoxelSize;

	uint Padding[2];
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
	uint		 NumNodes;
	uint		 Padding[3];
	VoxelOctreeNode Octree[];
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


layout(binding = 21, rgba16f) uniform image2D o_Image;
layout(binding = 22, r32f) uniform image2D o_DepthImage;


Ray g_CameraRay;

AABB OctreeNodeAABB(in VoxelOctreeNode node, float voxelSize)
{
	AABB result;
	result.Min = vec3(node.X, node.Y, node.Z) * voxelSize;
	result.Max = vec3(node.X + node.Size, node.Y + node.Size, node.Z + node.Size) * voxelSize;
	return result;
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
};


struct OctreeRaycastResult
{
	vec4 Color;
	bool Hit;
};

OctreeRaycastResult RaycastOctreeSteps(in Ray ray)
{
	OctreeRaycastResult result;
	result.Hit = false;

	 // Set initial step size for ray jumps
    float stepSize = 1.0;
	
    // Start at the origin of the ray
    vec3 position = ray.Origin;

	VoxelOctreeNode root = Octree[0];
	AABB rootAABB = OctreeNodeAABB(root, 6.0);

	if (!RayBoxIntersection(position, ray.Direction, rootAABB.Min, rootAABB.Max, stepSize))
		return result;

	//position += ray.Direction * (t + 1.0);
	for (int i = 0; i < 128; i++) 
	{
        // Advance the ray position along the ray direction by the current step size
        position += ray.Direction * stepSize;

        // Check if the current position is inside the octree bounds
		if (!PointInBox(position, rootAABB.Min, rootAABB.Max))
			break;

        // Traverse the octree to find the intersected node
        uint nodeIndex = 0; // Start from the root node
		float newStepSize = FLT_MAX;
        while (Octree[nodeIndex].IsLeaf == 0) 
		{
            // Check if the current position intersects with any child node AABBs
            bool hitChildNode = false;
			
			uint newNodeIndex = 0;
			float minDistance = FLT_MAX;
			float nearDist = FLT_MAX;
			float farDist = FLT_MAX;
            for (int c = 0; c < 8; c++) 
			{
                uint childIndex = Octree[nodeIndex].Children[c];
				VoxelOctreeNode child = Octree[childIndex];
				AABB aabb = OctreeNodeAABB(child, 6.0);
		
                if (RayBoxIntersection(position, ray.Direction, aabb.Min, aabb.Max, nearDist, farDist)) 
				{
					if (nearDist < minDistance)
					{
						newNodeIndex = childIndex; // Move to the intersected child node
						minDistance = nearDist;					
					}	
					if (farDist < newStepSize)
						newStepSize = farDist;
                    hitChildNode = true;
                }
            }
			//stepSize = minDistance;
			nodeIndex = newNodeIndex;			
            if (!hitChildNode) 
			{
                break; // Exit loop if no child node is intersected
            }
        }

		VoxelOctreeNode currentNode = Octree[nodeIndex];
        // Check if the intersected node is a leaf node
		
        if (currentNode.IsLeaf != 0) 
		{		
			if (currentNode.ColorIndex != 0)
			{
				result.Hit = true;
				// Perform additional operations for leaf nodes (e.g., shading, accumulating color)
				result.Color = VoxelToColor(ColorPallete[0][Octree[nodeIndex].ColorIndex]);
				break; // Exit loop after finding a hit		
			}
			else
			{
				stepSize = newStepSize;
			}
        }
    }




	return result;
}


OctreeRaycastResult RaycastOctree(in Ray ray)
{
	OctreeRaycastResult result;
	result.Hit = false;
	if (NumNodes == 0)
		return result;

	ivec2 textureIndex = ivec2(gl_GlobalInvocationID.xy);
	float t = 0.0;
	float currentDistance = imageLoad(o_DepthImage, textureIndex).r;
	VoxelOctreeNode stack[MAX_DEPTH * 10];
	int stackIndex = 0;
	stack[stackIndex++] = Octree[0];
	
	int pixelComplexity = 0;
	while (stackIndex > 0)
	{
		pixelComplexity++;
		stackIndex--;
		VoxelOctreeNode node = stack[stackIndex];
		float t = 0.0;
		
		if (node.IsLeaf == 0)
		{
			for (int c = 0; c < 8; c++)
			{
				VoxelOctreeNode child = Octree[node.Children[c]];
				AABB aabb = OctreeNodeAABB(child, 6.0);
				if (RayAABBOverlap(ray.Origin, ray.Direction, aabb.Min, aabb.Max))
				{
					stack[stackIndex++] = child;
				}
			}
		}
		else if (node.ColorIndex != 0)
		{
			result.Hit = true;
			result.Color = VoxelToColor(ColorPallete[0][node.ColorIndex]);
			return result;
		}
	}
	return result;
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

	float currentDistance = imageLoad(o_DepthImage, textureIndex).r;
	g_CameraRay = CreateRay(u_CameraPosition.xyz, textureIndex);

	if (u_Uniforms.UseOctree)
	{
		if (u_Uniforms.ShowAABB)
		{
			Ray ray = CreateRay(u_CameraPosition.xyz, Models[0].InverseTransform, textureIndex);
			OctreeRaycastResult result = RaycastOctreeSteps(ray);
			if (result.Hit)
			{
				imageStore(o_Image, textureIndex, result.Color);
			}
		}
		else
		{
			Ray ray = CreateRay(u_CameraPosition.xyz, Models[0].InverseTransform, textureIndex);
			OctreeRaycastResult result = RaycastOctree(ray);
			if (result.Hit)
			{
				imageStore(o_Image, textureIndex, result.Color);
			}
		}
	}
}