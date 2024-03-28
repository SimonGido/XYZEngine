#include "Resources/Shaders/Includes/Math.glsl"

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

	uint  Padding[2];
};

struct VoxelCompressedCell
{
	uint VoxelCount;
	uint VoxelOffset;
};

AABB VoxelAABB(ivec3 voxel, float voxelSize)
{
	AABB result;
	result.Min = vec3(
		float(voxel.x) * voxelSize, 
		float(voxel.y) * voxelSize, 
		float(voxel.z) * voxelSize
	);
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

Ray CreateRay(vec3 origin, in mat4 inverseModelSpace, vec2 coords, vec2 viewportSize, in mat4 inverseProjection, in mat4 inverseView)
{
	coords.x /= viewportSize.x;
	coords.y /= viewportSize.y;
	coords = coords * 2.0 - 1.0; // -1 -> 1
	vec4 target = inverseProjection * vec4(coords.x, -coords.y, 1, 1);
	Ray ray;
	ray.Origin = (inverseModelSpace * vec4(origin, 1.0)).xyz;

	ray.Direction = vec3(inverseModelSpace * inverseView * vec4(normalize(target.xyz / target.w), 0)); // World space
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

vec4 UINTColorToVec4(uint uintColor)
{
	vec4 color;
	color.x = bitfieldExtract(uintColor, 0, 8) / 255.0;
	color.y = bitfieldExtract(uintColor, 8, 8) / 255.0;
	color.z = bitfieldExtract(uintColor, 16, 8) / 255.0;
	color.w = bitfieldExtract(uintColor, 24, 8) / 255.0;

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

vec3 VoxelPosition(ivec3 voxel, float voxelSize)
{
	return vec3(voxel.x * voxelSize, voxel.y * voxelSize, voxel.z * voxelSize);
}