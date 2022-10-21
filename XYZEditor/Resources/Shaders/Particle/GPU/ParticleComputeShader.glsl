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
};


struct Particle
{
    // Data that are rendered
	vec4  TransformRow0;
    vec4  TransformRow1;
    vec4  TransformRow2;
    vec4  RenderColor;


    // Current state
    vec4  Color;
    vec4  Position;
    vec4  Rotation;
    vec4  Scale;
    vec4  Velocity;  
    
    float LifeRemaining;
    bool  Alive;
    uint  Padding0[2];

};

struct ParticleProperty
{
    // Spawn state
	vec4 StartPosition;
	vec4 StartColor;
	vec4 StartRotation;
	vec4 StartScale;
	vec4 StartVelocity;
	
	// If module enabled, end state
	vec4 EndColor;
	vec4 EndRotation;
	vec4 EndScale;
	vec4 EndVelocity;


	float LifeTime;

	uint  Padding[3];
};


layout(push_constant) uniform Uniform
{ 
    float Timestep;
    float Speed;
    uint  EmittedParticles;
    bool  Loop;
} u_Uniforms;


layout(std430, binding = 5) buffer buffer_DrawCommand // indirect
{
	DrawCommand Command;
};

layout (std430, binding = 6) buffer buffer_Particles
{
    Particle Particles[];
};

layout (std430, binding = 7) buffer buffer_ParticleProperties
{
    ParticleProperty ParticleProperties[];
};

layout (constant_id = 0) const bool COLOR_OVER_LIFE = false;
layout (constant_id = 1) const bool SCALE_OVER_LIFE = false;
layout (constant_id = 2) const bool VELOCITY_OVER_LIFE = false;
layout (constant_id = 3) const bool ROTATION_OVER_LIFE = false;


void SpawnParticle(uint id)
{
    Particles[id].Position      = ParticleProperties[id].StartPosition;
    Particles[id].Rotation      = ParticleProperties[id].StartRotation;
    Particles[id].Scale         = ParticleProperties[id].StartScale;
    Particles[id].Color         = ParticleProperties[id].StartColor;
    Particles[id].Velocity      = ParticleProperties[id].StartVelocity;
    Particles[id].LifeRemaining = ParticleProperties[id].LifeTime;
    Particles[id].Alive         = true;
}

bool ValidParticle(uint id)
{
    if (id >= u_Uniforms.EmittedParticles)
		return false;

    if (!Particles[id].Alive) // Particle is dead
    {
        if (u_Uniforms.Loop) // We loop so respawn particle
            SpawnParticle(id);
        else
            return false;
    }   
    return true;
}

void KillParticle(uint id)
{
    if (Particles[id].LifeRemaining <= 0.0)
    {
        Particles[id].Alive = false;
    }
}

void UpdateParticle(uint id)
{
    Particles[id].Position.xyz += Particles[id].Velocity.xyz * u_Uniforms.Timestep * u_Uniforms.Speed;
    Particles[id].LifeRemaining -= u_Uniforms.Timestep * u_Uniforms.Speed;
}

void UpdateRenderData(uint id, uint instanceIndex)
{
    mat4 transform = TranslationMatrix(Particles[id].Position.xyz, Particles[id].Scale.xyz)
                   * RotationMatrix(Particles[id].Rotation.xyz);

    Particles[instanceIndex].TransformRow0 = vec4(transform[0][0], transform[1][0], transform[2][0], transform[3][0]);
    Particles[instanceIndex].TransformRow1 = vec4(transform[0][1], transform[1][1], transform[2][1], transform[3][1]);
    Particles[instanceIndex].TransformRow2 = vec4(transform[0][2], transform[1][2], transform[2][2], transform[3][2]);
    Particles[instanceIndex].RenderColor = Particles[id].Color;
}



layout(local_size_x = 32, local_size_y = 32, local_size_z = 1) in;
void main(void)
{
    uint id = gl_GlobalInvocationID.x + gl_GlobalInvocationID.y * gl_NumWorkGroups.x * gl_WorkGroupSize.x;

    if (!ValidParticle(id))
        return;
   
    
    UpdateParticle(id);
    KillParticle(id); 
       
    
    if (Particles[id].Alive) // Particle is alive, write it to the buffer for rendering
    {  
        // Update particles here //
        float ratio = 1.0 - (Particles[id].LifeRemaining / ParticleProperties[id].LifeTime);   
        if (COLOR_OVER_LIFE)
        {
            Particles[id].Color = mix(ParticleProperties[id].StartColor, ParticleProperties[id].EndColor, ratio);
        }
        if (SCALE_OVER_LIFE)
        {
            Particles[id].Scale = mix(ParticleProperties[id].StartScale, ParticleProperties[id].EndScale, ratio);
        }
        if (VELOCITY_OVER_LIFE)
        {
            Particles[id].Velocity = mix(ParticleProperties[id].StartVelocity, ParticleProperties[id].EndVelocity, ratio);
        }
        if (ROTATION_OVER_LIFE)
        {
             Particles[id].Rotation = mix(ParticleProperties[id].StartRotation, ParticleProperties[id].EndRotation, ratio);   
        }
        ///////////////////////////
        uint instanceIndex = atomicAdd(Command.InstanceCount, 1);
        UpdateRenderData(id, instanceIndex);
    }
}