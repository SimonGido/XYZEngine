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

	ray.Direction = vec3(u_InverseView * u_Uniforms.Transform * vec4(normalize(vec3(target) / target.w), 0)); // World space
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


bool IsInsideChunk(ivec3 voxel)
{
	vec3 current_voxel = vec3(voxel.x, voxel.y, voxel.z);
	return ((current_voxel.x < float(u_Uniforms.Width ) && current_voxel.x > 0)
		 && (current_voxel.y < float(u_Uniforms.Height) && current_voxel.y > 0)
		 && (current_voxel.z < float(u_Uniforms.Depth)  && current_voxel.z > 0));
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

vec4 RayMarch(vec3 rayOrig, vec3 rayDir)
{
	vec4 defaultColor = vec4(0.3, 0.2, 0.7, 1.0);
	vec3 origin = rayOrig;
	vec3 direction = rayDir;

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
	vec3 normal;
	uint voxel = 0;
	uint i = 0;

	bool first = true;
	vec4 result = defaultColor;
	do 
	{
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
		
		// Is inside voxel area
		if (IsInsideChunk(current_voxel))
		{
			voxel = Voxels[Index3D(current_voxel)];
			if (voxel != 0)
			{
				float light = dot(-u_LightDirection.xyz, normal);
				vec4 voxelColor = VoxelToColor(voxel);
				vec3 color = voxelColor.rgb * u_LightColor.rgb * light;
					
				if (voxelColor.a < 1.0)
				{
					voxel = 0;
				}

				if (first)
				{
					first = false;
					result = vec4(color.rgb, voxelColor.a);
				}
				else
				{
					result.rgb = mix(result.rgb, color, 1.0 - result.a);
				}
			}
		}
		i += 1;
	} 
	while (voxel == 0 && i < u_Uniforms.MaxTraverse);
	
	if (voxel == 0 && first == false) // We did not hit opaque voxel => blend with background color
	{
		result.rgb = mix(result.rgb, defaultColor.rgb, 1.0 - result.a);
	}
	return result;

	return defaultColor;
}


layout(local_size_x = 32, local_size_y = 32, local_size_z = 1) in;
void main() 
{
	Ray ray = CreateRay(gl_GlobalInvocationID.xy);
	vec4 color = RayMarch(ray.Origin, ray.Direction);
	imageStore(o_Image, ivec2(gl_GlobalInvocationID), color);
}