//#type compute
#version 460
#extension GL_EXT_shader_8bit_storage : enable

#include "Resources/Shaders/Includes/Random.glsl"
#include "Resources/Shaders/Includes/PBR.glsl"

//https://github.com/RiscadoA/voxel-platformer/blob/master/src/vpg/gl/renderer.cpp

#define TILE_SIZE 16

const float FLT_MAX = 3.402823466e+38;
const float EPSILON = 0.01;

layout (std140, binding = 16) uniform Scene
{
	// Camera info
	mat4 u_InverseProjection;
	mat4 u_InverseView;	
	vec4 u_CameraPosition;
	vec4 u_ViewportSize;

	// Light info
	DirectionalLight u_DirectionalLight;
	uint MaxTraverse;
};

layout(push_constant) uniform SSAO
{
	float SampleRadius; 
    float Intensity; 
    float Scale; 
    float Bias;
    int   NumIterations;

} u_Uniforms;

layout(binding = 0, rgba32f) uniform image2D o_Image;
layout(binding = 1, rgba32f) uniform image2D u_NormalImage;
layout(binding = 2, rgba32f) uniform image2D u_PositionImage;


vec3 GetPosition(ivec2 coord) 
{
    return imageLoad(u_PositionImage, coord).xyz;
}

vec3 GetNormal(ivec2 coord) 
{
    return normalize(imageLoad(u_NormalImage, coord).xyz * 2.0 - 1.0);
}

vec2 GetRandom(vec2 coord)
{
    vec2 result;
    result.x = Random(-1.0, 1.0, coord.x);
    result.y = Random(-1.0, 1.0, coord.y);
    return result;
}

float DoAmbientOcclusion(vec2 tuv, vec2 uv, vec3 p, in vec3 cnorm) 
{
    vec2 sumUV = tuv + uv;
    ivec2 posCoord = ivec2(sumUV * u_ViewportSize.xy);

    vec3 diff = GetPosition(posCoord) - p; 
    vec3 v = normalize(diff); 
    float d = length(diff) * u_Uniforms.Scale; 
    return max(0.0,dot(cnorm,v) - u_Uniforms.Bias) * (1.0/ (1.0 + d)) * u_Uniforms.Intensity;
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
	const vec2 vec[4] = {
        vec2(1,0),
        vec2(-1,0), 
        vec2(0,1),
        vec2(0,-1)
    };
    
    vec2 uv = gl_GlobalInvocationID.xy / u_ViewportSize.xy;

    vec3 p = GetPosition(ivec2(gl_GlobalInvocationID.xy)); 
    vec3 n = GetNormal(ivec2(gl_GlobalInvocationID.xy)); 
    vec2 rand = GetRandom(gl_GlobalInvocationID.xy); 
    float ao = 0.0; 
    float radius = u_Uniforms.SampleRadius / p.z; 
    float coordMultiplier = 0.707;

    //**SSAO Calculation**// 
    for (int j = 0; j < u_Uniforms.NumIterations; ++j) 
    {
        vec2 coord1 = reflect(vec[j % 4], rand) * radius; 
        vec2 coord2 = vec2(
            coord1.x * coordMultiplier - coord1.y * coordMultiplier, 
            coord1.x * coordMultiplier + coord1.y * coordMultiplier
        ); 
        

        ao += DoAmbientOcclusion(uv, coord1 * 0.25, p, n); 
        ao += DoAmbientOcclusion(uv, coord2 * 0.5, p, n); 
        ao += DoAmbientOcclusion(uv, coord1 * 0.75, p, n); 
        ao += DoAmbientOcclusion(uv, coord2, p, n); 
    }

    vec4 origColor = imageLoad(o_Image, ivec2(gl_GlobalInvocationID).xy);
    vec3 dirLight = CalculateDirLights(p, origColor.rgb, n);
    ao = 1.0 - (ao / float(u_Uniforms.NumIterations)); 
    origColor.rgb = dirLight * ao;
    imageStore(o_Image, ivec2(gl_GlobalInvocationID.xy), origColor);
}