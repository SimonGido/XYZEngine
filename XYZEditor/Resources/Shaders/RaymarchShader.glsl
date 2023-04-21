//#type compute
#version 460

#include "Resources/Shaders/Includes/Math.glsl"

layout(push_constant) uniform Uniform
{ 
	mat4  Transform;
	uint  MaxTraverse;
	uint  Width;
	uint  Height;
	uint  Depth;
	float VoxelSize;

} u_Uniforms;

layout (std140, binding = 16) uniform Scene
{
	// Camera info
	mat4 u_InverseProjection;
	mat4 u_InverseView;	
	vec4 u_CameraPosition;
	vec4 u_ViewportSize;


	//vec4 u_RayOrigin;

	// Light info
	vec4 u_LightDirection;
	vec4 u_LightColor;
};


layout(std430, binding = 17) buffer buffer_Voxels
{
	uint Voxels[];
};

layout(binding = 18, rgba32f) uniform image2D o_Image;

struct Ray
{
	vec3 Origin;
	vec3 Direction;
};

Ray CreateRay(vec2 coords)
{
	coords.x /= u_ViewportSize.x;
	coords.y /= u_ViewportSize.y;
	coords = coords * 2.0 - 1.0; // -1 -> 1
	vec4 target = u_InverseProjection * vec4(coords.x, -coords.y, 1, 1);
	vec4 rayOrigin = u_CameraPosition;
	
	Ray ray;
	ray.Origin = (u_Uniforms.Transform * rayOrigin).xyz;

	ray.Direction = vec3(u_Uniforms.Transform * u_InverseView * vec4(normalize(vec3(target) / target.w), 0)); // World space
	ray.Direction = normalize(ray.Direction);

	return ray;
}


uint Index3D(int x, int y, int z)
{
	return x + u_Uniforms.Width * (y + u_Uniforms.Depth * z);
}

uint Index3D(ivec3 index)
{
	return Index3D(index.x, index.y, index.z);
}


bool IsValidVoxel(ivec3 voxel)
{
	return ((voxel.x < u_Uniforms.Width  && voxel.x > 0)
		 && (voxel.y < u_Uniforms.Height && voxel.y > 0)
		 && (voxel.z < u_Uniforms.Depth  && voxel.z > 0));
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

struct RaymarchResult
{
	vec4 color;
	uint alpha;
	vec3 t_max;
	ivec3 current_voxel;
	uint traverseCount;
};


RaymarchResult RayMarch(vec3 t_max, vec3 t_delta, ivec3 current_voxel, ivec3 step, uint maxTraverses)
{
	RaymarchResult result;
	result.color = vec4(0.3, 0.2, 0.7, 1.0);
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
			uint voxel = Voxels[Index3D(current_voxel)];
			if (voxel != 0)
			{
				float light = dot(-u_LightDirection.xyz, normal);
				vec4 voxelColor = VoxelToColor(voxel);
				vec3 color = voxelColor.rgb * u_LightColor.rgb * light;
				result.alpha = VoxelAlpha(voxel);
				result.color = vec4(color.rgb, voxelColor.a);
				break;
			}
		}
	}
	result.traverseCount = i;
	result.t_max = t_max;
	result.current_voxel = current_voxel;

	return result;
}

vec4 RayMarch(vec3 origin, vec3 direction)
{
	vec4 defaultColor = vec4(0.3, 0.2, 0.7, 1.0);
	ivec3 current_voxel = ivec3(floor(origin / u_Uniforms.VoxelSize));
	
	ivec3 step = ivec3(
		(direction.x > 0.0) ? 1 : -1,
		(direction.y > 0.0) ? 1 : -1,
		(direction.z > 0.0) ? 1 : -1
	);
	vec3 next_boundary = vec3(
		float((step.x > 0) ? current_voxel.x + 1 : current_voxel.x) * u_Uniforms.VoxelSize,
		float((step.y > 0) ? current_voxel.y + 1 : current_voxel.y) * u_Uniforms.VoxelSize,
		float((step.z > 0) ? current_voxel.z + 1 : current_voxel.z) * u_Uniforms.VoxelSize
	);

	vec3 t_max = (next_boundary - origin) / direction; // we will move along the axis with the smallest value
	vec3 t_delta = u_Uniforms.VoxelSize / direction * vec3(step);	


	uint remainingTraverses = u_Uniforms.MaxTraverse;
	RaymarchResult hit = RayMarch(t_max, t_delta, current_voxel, step, remainingTraverses);
	vec4 result = hit.color;
	remainingTraverses -= hit.traverseCount;

	bool opaqueHit = false;
	while (remainingTraverses != 0)
	{
		if (hit.alpha == 1)
		{
			opaqueHit = true;
			break;
		}
		hit = RayMarch(hit.t_max, t_delta, hit.current_voxel, step, remainingTraverses);
		result.rgb = mix(result.rgb, hit.color.rgb, 1.0 - result.a);
		remainingTraverses -= hit.traverseCount;
	}

	if (!opaqueHit) // We did not hit opaque voxel => blend with background color
	{
		result.rgb = mix(result.rgb, defaultColor.rgb, 1.0 - result.a);
	}
	return result;
}



layout(local_size_x = 32, local_size_y = 32, local_size_z = 1) in;
void main() 
{
	Ray ray = CreateRay(gl_GlobalInvocationID.xy);
	vec4 color = RayMarch(ray.Origin, ray.Direction);
	imageStore(o_Image, ivec2(gl_GlobalInvocationID), color);
}