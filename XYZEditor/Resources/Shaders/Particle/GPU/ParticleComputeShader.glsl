//#type compute
#version 460


struct DrawCommand 
{
	uint Count;         
	uint InstanceCount; 
	uint FirstIndex;    
	uint BaseVertex;    
	uint BaseInstance;  
};

struct Particle
{
	vec4    TransformRow0;
    vec4    TransformRow1;
    vec4    TransformRow2;
    vec4    Color;
    vec2    Velocity;   
    float   LifeRemaining;
    bool    Alive;
};

// Data used when particle is spawned
struct ParticleProperties
{
    vec4 Position;
    vec4 Velocity;
    vec4 Color;
};

layout(push_constant) uniform Uniform
{ 
    vec4  EndColor;
    vec3  EndRotation;
    vec3  EndSize;
    float LifeTime;
    float Time;
    float Speed;
    int   MaxParticles;
    int   ParticlesEmitted;

} u_Uniforms;


layout (std140, binding = 15) buffer buffer_State
{
	uint DeadCount;
};


layout(std430, binding = 1) buffer buffer_DrawCommand // indirect
{
	DrawCommand Command;
};


layout(local_size_x = 32, local_size_y = 32, local_size_z = 1) in;
void main(void)
{
    uint id = gl_GlobalInvocationID.x + gl_GlobalInvocationID.y * gl_NumWorkGroups.x * gl_WorkGroupSize.x;
    uint deadCount = DeadCount;

    if (id > u_Uniforms.MaxParticles || id < deadCount)
		return;

    Command.Count = 6;
	Command.InstanceCount = u_Uniforms.ParticlesEmitted - deadCount;
	Command.FirstIndex = 0;
	Command.BaseVertex = 0;
	Command.BaseInstance = deadCount;
}