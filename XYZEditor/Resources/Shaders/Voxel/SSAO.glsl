//#type compute
#version 460
#extension GL_EXT_shader_8bit_storage : enable
#include "Resources/Shaders/Includes/Math.glsl"

//https://github.com/RiscadoA/voxel-platformer/blob/master/src/vpg/gl/renderer.cpp

#define TILE_SIZE 16

const float FLT_MAX = 3.402823466e+38;
const float EPSILON = 0.01;
const uint OPAQUE = 255;

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




layout(binding = 0, rgba32f) uniform image2D o_Image;
layout(binding = 1, rgb32f) uniform image2D u_NormalImage;
layout(binding = 2, rgb32f) uniform image2D u_PositionImage;

layout(local_size_x = TILE_SIZE, local_size_y = TILE_SIZE, local_size_z = 1) in;
void main() 
{
	ivec2 textureIndex = ivec2(gl_GlobalInvocationID.xy);
	
	vec3 frag_pos = imageLoad(u_PositionImage, textureIndex).xyz;
    vec3 normal = normalize(imageLoad(u_NormalImage, textureIndex).xyz);

    vec3 random_vec = RandomValue(gl_GlobalInvocationID.xy, gl_WorkGroupID.x);

    vec3 tangent = normalize(random_vec - normal * dot(random_vec, normal));
    vec3 bitangent = cross(normal, tangent);
    mat3 tbn = mat3(tangent, bitangent, normal);
            
    float occlusion = 0.0;
    for (int i = 0; i < NUM_SAMPLES; ++i) {
        vec3 sample_pos = tbn * u_Uniforms.Samples[i];
        sample_pos = frag_pos + sample_pos * radius;
                
        vec4 offset = projection * vec4(sample_pos, 1.0);
        offset.xy /= offset.w;
        offset.xy = offset.xy * 0.5 + 0.5;
                
        float sample_depth = texture(position_tex, offset.xy).z;
        float range_check = smoothstep(0.0, 1.0, radius / abs(frag_pos.z - sample_depth));
        occlusion += (sample_depth >= sample_pos.z + bias ? 1.0 : 0.0) * range_check;
    }
            
    occlusion = 1.0 - occlusion / NUM_SAMPLES;
    occlusion = pow(occlusion, magnitude);
    frag_color = constrast * (occlusion - 0.5) + 0.5;

}