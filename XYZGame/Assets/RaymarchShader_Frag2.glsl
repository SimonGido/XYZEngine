#version 450

layout(location = 0) out vec4 o_Color;



struct VertexOutput
{
	vec3 Ray;
	vec3 RayOrigin;

	vec3 LightDirection;
	vec3 LightColor;
};

struct HitResult
{
	vec3 Normal;
	uint Color;
};

struct Voxel
{
	uint Color;
	uint Distance;
};

layout(location = 0) in VertexOutput v_Input;

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


float maxcomp(in vec3 p) { return max(p.x, max(p.y, p.z)); }
double maxcomp(in dvec3 p) { return max(p.x, max(p.y, p.z)); }
float maxcomp(in vec2 p) { return max(p.x, p.y); }
double maxcomp(in dvec2 p) { return max(p.x, p.y); }

float mincomp(in vec3 p) { return min(p.x, min(p.y, p.z)); }
double mincomp(in dvec3 p) { return min(p.x, min(p.y, p.z)); }

// Voxel colors
layout(std430, binding = 3) buffer buffer_Voxels
{
	Voxel Voxels[];
};


uint Index3D(int x, int y, int z)
{
	return x + u_Width * (y + u_Depth * z);
}

uint Index3D(ivec3 index)
{
	return Index3D(index.x, index.y, index.z);
}

bool IsInsideChunk(ivec3 voxel)
{
	vec3 current_voxel = vec3(voxel.x, voxel.y, voxel.z);
	return ((current_voxel.x < float(u_Width ) && current_voxel.x > 0)
		 && (current_voxel.y < float(u_Height) && current_voxel.y > 0)
		 && (current_voxel.z < float(u_Depth)  && current_voxel.z > 0));
}


vec4 VoxelToColor(uint voxel)
{
	vec4 color;
	color.x = bitfieldExtract(voxel, 0, 8);
	color.y = bitfieldExtract(voxel, 8, 8);
	color.z = bitfieldExtract(voxel, 16, 8);
	color.w = bitfieldExtract(voxel, 24, 8);
	color /= 255.0;
	return color;
}

vec4 HitResultToColor(in HitResult hit)
{
	float light = dot(-v_Input.LightDirection, hit.Normal);
	vec4 voxelColor = VoxelToColor(hit.Color);
	vec3 color = voxelColor.rgb * v_Input.LightColor * light;
	return vec4(color, voxelColor.a);
}

bool RayMarch(out HitResult result, inout ivec3 currentVoxel, ivec3 step, inout vec3 tMax, inout vec3 tDelta, inout uint numTraverses)
{
	result.Color = 0;
	do 
	{
		if (tMax.x < tMax.y && tMax.x < tMax.z) 
		{
			result.Normal = vec3(float(-step.x), 0.0, 0.0);
			tMax.x += tDelta.x;
			currentVoxel.x += step.x;
		}
		else if (tMax.y < tMax.z) 
		{
			result.Normal = vec3(0.0, float(-step.y), 0.0);
			tMax.y += tDelta.y;
			currentVoxel.y += step.y;			
		}
		else 
		{
			result.Normal = vec3(0.0, 0.0, float(-step.z));
			tMax.z += tDelta.z;
			currentVoxel.z += step.z;		
		}
		
		// Is inside voxel area
		if (IsInsideChunk(currentVoxel))
		{
			result.Color = Voxels[Index3D(currentVoxel)].Color;
			if (result.Color != 0)
				return true;
		}
		numTraverses += 1;
	} 
	while (result.Color == 0 && numTraverses < u_MaxTraverse);

	return false;
}

const float eps = 0.0005;

vec4 HitColor(vec3 rayOrig, vec3 dir)
{
	uint voxelGridSize = u_Width;
	ivec3 p0 = ivec3(floor(rayOrig / u_VoxelSize));
	vec4 result = u_BackgroundColor;

	float endT = mincomp(max((vec3(voxelGridSize) - p0) / dir, -p0 / dir));

	vec3 p0abs = (1 - step(0, dir)) * voxelGridSize + sign(dir) * p0;
	vec3 dirAbs = abs(dir);

	uint numIter = 0;
	float t = 0;
	while (t <= endT && numIter < u_MaxTraverse)
	{
		// Next point to check
		vec3 p = p0 + dir * t;
		ivec3 index = ivec3(floor(p / (u_VoxelSize * voxelGridSize)));
		Voxel voxel = Voxels[Index3D(index)];

		// Stop if voxel is solid
		if (voxel.Color != 0) 
		{
			return VoxelToColor(voxel.Color);
		}
		vec3 pAbs = p0abs + dirAbs * t;
		vec3 deltas = (1 - fract(p)) / dirAbs;
		t += max(mincomp(deltas), eps) + voxel.Distance;

		numIter++;
	}

	return result;
}



void main() 
{
	vec4 color = HitColor(v_Input.RayOrigin, v_Input.Ray);
	o_Color = color;
}