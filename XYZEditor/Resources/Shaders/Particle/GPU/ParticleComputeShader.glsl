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
    // Data that are rendered
	vec4    TransformRow0;
    vec4    TransformRow1;
    vec4    TransformRow2;
    vec4    Color;    
};

struct ParticleProperty
{
    // Spawn state
    vec4 StartPosition;
    vec4 StartVelocity;
    vec4 StartColor;

    // Current state
    vec4  Position;
    vec4  Color;
    vec3  Velocity;   
    float LifeRemaining;
    bool  Alive;
};

layout(push_constant) uniform Uniform
{ 
    vec4  EndColor;
    vec3  EndRotation;
    vec3  EndSize;
    float LifeTime;
    float Timestep;
    float Speed;
    uint  MaxParticles;
    uint  ParticlesEmitted;
    int   Loop;

} u_Uniforms;


layout (std140, binding = 15) buffer buffer_State
{
	uint InstanceCount;
};

layout (std140, binding = 16) buffer buffer_Particles
{
    Particle Particles[];
};

layout (std140, binding = 17) buffer buffer_ParticleProperties
{
    ParticleProperty ParticleProperties[];
};

layout(std430, binding = 18) buffer buffer_DrawCommand // indirect
{
	DrawCommand Command;
};

mat4 TranslationMatrix(vec3 translation)
{
    return mat4(
        vec4(1.0, 0.0, 0.0, 0.0),
        vec4(0.0, 1.0, 0.0, 0.0),
        vec4(0.0, 0.0, 1.0, 0.0),
        vec4(translation, 1.0));
}


void SpawnParticle(uint id)
{
    ParticleProperties[id].Position      = ParticleProperties[id].StartPosition;
    ParticleProperties[id].Color         = ParticleProperties[id].StartColor;
    ParticleProperties[id].Velocity      = ParticleProperties[id].StartVelocity.xyz;
    ParticleProperties[id].LifeRemaining = u_Uniforms.LifeTime;
    ParticleProperties[id].Alive         = true;
}

bool ValidParticle(uint id)
{
    if (id >= u_Uniforms.MaxParticles)
		return false;

    if (!ParticleProperties[id].Alive) // Particle is dead
    {
        if (u_Uniforms.Loop != 0) // We loop so respawn particle
            SpawnParticle(id);
        else
            return false;
    }   
    return true;
}

void KillParticle(uint id)
{
    if (ParticleProperties[id].LifeRemaining <= 0.0)
    {
        ParticleProperties[id].Alive = false;
        if (u_Uniforms.Loop != 0) // We loop so respawn particle
            SpawnParticle(id);
    }
}

void UpdateParticle(uint id)
{
    ParticleProperties[id].Position.xyz += ParticleProperties[id].Velocity;
    ParticleProperties[id].LifeRemaining -= u_Uniforms.Timestep;
}

void UpdateRenderData(uint id, uint instanceIndex)
{
    mat4 transform = TranslationMatrix(ParticleProperties[id].Position.xyz);
    Particles[instanceIndex].TransformRow0 = vec4(transform[0][0], transform[1][0], transform[2][0], transform[3][0]);
    Particles[instanceIndex].TransformRow1 = vec4(transform[0][1], transform[1][1], transform[2][1], transform[3][1]);
    Particles[instanceIndex].TransformRow2 = vec4(transform[0][2], transform[1][2], transform[2][2], transform[3][2]);
    Particles[instanceIndex].Color         = ParticleProperties[id].StartColor;
}

layout(local_size_x = 32, local_size_y = 32, local_size_z = 1) in;
void main(void)
{
    uint id = gl_GlobalInvocationID.x + gl_GlobalInvocationID.y * gl_NumWorkGroups.x * gl_WorkGroupSize.x;
    if (!ValidParticle(id))
        return;
   
    // Update particles here
    UpdateParticle(id);
    KillParticle(id);

    if (ParticleProperties[id].Alive) // Particle is alive, write it to the buffer for rendering
    {  
        uint instanceIndex = atomicAdd(InstanceCount, 1);
        UpdateRenderData(id, instanceIndex);
	    atomicMax(Command.InstanceCount, instanceIndex + 1);
    }

    Command.Count = 6;
	Command.FirstIndex = 0;
	Command.BaseVertex = 0;
	Command.BaseInstance = 0;
}