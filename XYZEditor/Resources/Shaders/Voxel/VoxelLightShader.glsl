//#type compute
#version 460

#include "Resources/Shaders/Includes/Math.glsl"
#include "Resources/Shaders/Includes/PBR.glsl"

#define TILE_SIZE 16

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

layout(binding = 1, rgba32f) uniform image2D o_Image;
layout(binding = 2, rgba32f) uniform image2D o_Normal;
layout(binding = 3, rgba32f) uniform image2D o_Position;

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
	ivec2 location = ivec2(gl_GlobalInvocationID.xy);

	vec4 color = imageLoad(o_Image, location);

	vec3 normal = imageLoad(o_Normal, location).rgb;
	vec3 position = imageLoad(o_Position, location).xyz;

	color.rgb = CalculateDirLights(position, color.rgb, normal);
	
	imageStore(o_Image, location, color);
}