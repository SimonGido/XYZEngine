//#type compute
#version 460
#include "Resources/Shaders/Includes/Math.glsl"
struct DrawCommand
{
	uint Count;
	uint InstanceCount;
	uint FirstIndex;
	uint BaseVertex;
	uint BaseInstance;
	uint Padding[3];
};
struct Output
{
	vec4 TransformRow0;
	vec4 TransformRow1;
	vec4 TransformRow2;
	vec4 Color;
};
struct Input
{
	vec4 StartPosition;
	vec4 StartColor;
	vec4 StartRotation;
	vec4 StartScale;
	vec4 StartVelocity;
	vec4 EndColor;
	vec4 EndRotation;
	vec4 EndScale;
	vec4 EndVelocity;
	vec4 Position;
	float LifeTime;
	float LifeRemaining;
	uint Padding[2];
};
layout(push_constant) uniform Uniform
{
	uint CommandCount;
	float Timestep;
	float Speed;
	uint EmittedParticles;
	bool Loop;
}u_Uniforms;
layout(std430, binding = 5) buffer buffer_DrawCommand
{
	DrawCommand Command[];

};
layout(std430, binding = 6) buffer buffer_Output
{
	Output Outputs[];

};
layout(std430, binding = 7) buffer buffer_Input
{
	Input Inputs[];

};
layout(local_size_x = 32, local_size_y = 32, local_size_z = 1) in;
void main(void)
{
	uint id = gl_GlobalInvocationID.x + gl_GlobalInvocationID.y * gl_NumWorkGroups.x * gl_WorkGroupSize.x;
	if (id >= u_Uniforms.EmittedParticles)
		return;
}