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
	Update(id);
}