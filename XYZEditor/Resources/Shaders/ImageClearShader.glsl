//#type compute
#version 460

layout(push_constant) uniform clearColors
{
	vec4 ClearColor;
	float Depth;

} u_Uniforms;



layout(binding = 19, rgba32f) uniform image2D o_Image;
layout(binding = 20, r32f) uniform image2D o_DepthImage;

layout(local_size_x = 32, local_size_y = 32, local_size_z = 1) in;
void main() 
{	
	imageStore(o_Image, ivec2(gl_GlobalInvocationID), u_Uniforms.ClearColor);
	imageStore(o_DepthImage, ivec2(gl_GlobalInvocationID), vec4(u_Uniforms.Depth));
}