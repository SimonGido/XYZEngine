//#type compute
#version 460

#include "Resources/Shaders/Includes/Math.glsl"
#include "Resources/Shaders/Includes/PBR.glsl"

struct DrawCommand 
{
	uint Count;         
	uint InstanceCount; 
	uint FirstIndex;    
	uint BaseVertex;    
	uint BaseInstance;  

    uint Padding[3];
};

struct Transform
{
    vec4  TransformRow0;
    vec4  TransformRow1;
    vec4  TransformRow2;
};

struct Particle
{
    // Data that are rendered
	Transform Tr;
    vec4      Color;
};

struct ParticleState
{
    vec4  Color;
    vec4  Rotation;
    vec3  Scale;
    vec3  Velocity;  
    bool  Alive;
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
    
  
    vec4  Position;
	float LifeTime;
    float LifeRemaining;

	uint  Padding[2];
};


layout(push_constant) uniform Uniform
{ 
    float Timestep;
    float Speed;
    uint  EmittedParticles;
    bool  Loop;

} u_Uniforms;

layout(std140, binding = 2) buffer buffer_PointLightsData
{
	uint NumberPointLights;
	PointLight PointLights[MAX_POINT_LIGHTS];
};

layout(std430, binding = 5) buffer buffer_DrawCommand // indirect
{
	DrawCommand Command[];
};

layout (std430, binding = 6) buffer buffer_Particles
{
    Particle Particles[];
};

layout (std430, binding = 7) buffer buffer_ParticleProperties
{
    ParticleProperty ParticleProperties[];
};

layout(std430, binding = 8) buffer buffer_CommandData
{
	uint CommandCount;
    uint Padding[3];
	Transform Transforms[];
};

layout (constant_id = 0) const bool COLOR_OVER_LIFE = false;
layout (constant_id = 1) const bool SCALE_OVER_LIFE = false;
layout (constant_id = 2) const bool VELOCITY_OVER_LIFE = false;
layout (constant_id = 3) const bool ROTATION_OVER_LIFE = false;
layout (constant_id = 4) const bool SPAWN_LIGHTS = false;


shared uint pointLightsCount;

float LifeProgress(uint id)
{
    return 1.0 - (ParticleProperties[id].LifeRemaining / ParticleProperties[id].LifeTime);
}

void UpdateRenderData(in ParticleState state, uint id, uint instanceIndex)
{
    mat4 transform = TranslationMatrix(ParticleProperties[id].Position.xyz, state.Scale)
                   * RotationMatrix(state.Rotation);

    Particles[instanceIndex].Tr.TransformRow0 = vec4(transform[0][0], transform[1][0], transform[2][0], transform[3][0]);
    Particles[instanceIndex].Tr.TransformRow1 = vec4(transform[0][1], transform[1][1], transform[2][1], transform[3][1]);
    Particles[instanceIndex].Tr.TransformRow2 = vec4(transform[0][2], transform[1][2], transform[2][2], transform[3][2]);
    Particles[instanceIndex].Color = state.Color;
}


void RespawnParticle(uint id)
{
    ParticleProperties[id].LifeRemaining = ParticleProperties[id].LifeTime;
    ParticleProperties[id].Position = ParticleProperties[id].StartPosition;  
}


void SpawnLight(uint id, uint lightIndex, uint commandIndex, in ParticleState state)
{
    if (lightIndex < MAX_POINT_LIGHTS)
    {
        mat4 particleTransform = TranslationMatrix(ParticleProperties[id].Position.xyz, state.Scale)
                   * RotationMatrix(state.Rotation);
        

        mat4 transform = TransformFromRows(
            Transforms[commandIndex].TransformRow0,
            Transforms[commandIndex].TransformRow1,
            Transforms[commandIndex].TransformRow2
        ) * particleTransform;


        PointLights[lightIndex] = PointLights[0];
        PointLights[lightIndex].Position = transform[3].xyz;
        atomicAdd(NumberPointLights, 1);
    }
}

void CommandsUpdate(uint id, out uint instanceIndex, in ParticleState state)
{
    for (uint i = 0; i < CommandCount; ++i)
    {
        instanceIndex = atomicAdd(Command[i].InstanceCount, 1);
        if (SPAWN_LIGHTS)
        {
            uint lightIndex = atomicAdd(pointLightsCount, 1);
            SpawnLight(id, lightIndex, i, state);
        }
    }
}


void UpdateParticle(uint id)
{
    float lifeProgress = LifeProgress(id);
 
    ParticleState state;
    state.Alive = ParticleProperties[id].LifeRemaining > 0.0;
    // Particle is alive initialize it
    if (state.Alive)
    {       
        state.Color    = ParticleProperties[id].StartColor;
        state.Scale    = ParticleProperties[id].StartScale.xyz;
        state.Velocity = ParticleProperties[id].StartVelocity.xyz;
        state.Rotation = ParticleProperties[id].StartRotation;

        if (COLOR_OVER_LIFE)
        {
            state.Color = mix(ParticleProperties[id].StartColor, ParticleProperties[id].EndColor, lifeProgress);
        }
        if (SCALE_OVER_LIFE)
        {
            state.Scale = mix(ParticleProperties[id].StartScale, ParticleProperties[id].EndScale, lifeProgress).xyz;
        }
        if (VELOCITY_OVER_LIFE)
        {
            state.Velocity = mix(ParticleProperties[id].StartVelocity, ParticleProperties[id].EndVelocity, lifeProgress).xyz;
        }
        if (ROTATION_OVER_LIFE)
        {
            state.Rotation = mix(ParticleProperties[id].StartRotation, ParticleProperties[id].EndRotation, lifeProgress);   
        }
        

        ParticleProperties[id].Position.xyz += state.Velocity * u_Uniforms.Timestep * u_Uniforms.Speed;
        ParticleProperties[id].LifeRemaining -= u_Uniforms.Timestep;

        uint instanceIndex = 0;
        CommandsUpdate(id, instanceIndex, state);
        UpdateRenderData(state, id, instanceIndex);
    }
    else if (u_Uniforms.Loop)
    {
        RespawnParticle(id);
    }
}

bool ValidParticle(uint id)
{
    if (id >= u_Uniforms.EmittedParticles)
		return false; 
    return true;
}


layout(local_size_x = 32, local_size_y = 32, local_size_z = 1) in;
void main(void)
{
    if (gl_LocalInvocationIndex == 0)   
        pointLightsCount = NumberPointLights;
    barrier();

    uint id = gl_GlobalInvocationID.x + gl_GlobalInvocationID.y * gl_NumWorkGroups.x * gl_WorkGroupSize.x;
    
    if (!ValidParticle(id))
        return;

    UpdateParticle(id);
}