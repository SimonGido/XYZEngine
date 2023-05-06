//#type compute
#version 460

layout(push_constant) uniform clearColors
{
	vec4 ClearColor;
	float Depth;

} u_Uniforms;

#define TILE_SIZE 16

layout(binding = 0, rgba16f) uniform image2D o_Image;
layout(binding = 1, r32f) uniform image2D o_DepthImage;

layout(local_size_x = TILE_SIZE, local_size_y = TILE_SIZE, local_size_z = 1) in;
void main() 
{	
	ivec2 location = ivec2(gl_GlobalInvocationID.xy);

	imageStore(o_Image, location, u_Uniforms.ClearColor);
	imageStore(o_DepthImage, location, vec4(u_Uniforms.Depth));
}