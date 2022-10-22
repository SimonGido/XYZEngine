//#type compute
#version 460
#include "Resources/Shaders/Includes/Math.glsl"
#include "Resources/Shaders/Includes/Random.glsl"



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

layout (constant_id = 0) const uint MAX_PARTICLES = 1024;


layout (std430, binding = 6) buffer buffer_ParticleInitializeData
{
    vec4 StartPositionMin;
	vec4 StartPositionMax;

    vec4 StartColorMin;
	vec4 StartColorMax;	

	vec4 StartRotationMin;
	vec4 StartRotationMax;

	vec4 StartScaleMin;
	vec4 StartScaleMax;

	vec4 StartVelocityMin;
	vec4 StartVelocityMax;

	vec4 EndColorMin;
	vec4 EndColorMax;

	vec4 EndRotationMin;
	vec4 EndRotationMax;	

	vec4 EndScaleMin;
	vec4 EndScaleMax;

	vec4 EndVelocityMin;
	vec4 EndVelocityMax;

	float LifeTimeMin;
	float LifeTimeMax;

	uint Padding[2];
};

layout (std430, binding = 7) buffer buffer_ParticleProperties
{
	ParticleProperty ParticleProperties[MAX_PARTICLES];
};


bool ValidParticle(uint id)
{
    if (id >= MAX_PARTICLES)
		return false; 
    return true;
}

layout(local_size_x = 32, local_size_y = 32, local_size_z = 1) in;
void main(void)
{
    uint id = gl_GlobalInvocationID.x + gl_GlobalInvocationID.y * gl_NumWorkGroups.x * gl_WorkGroupSize.x;

    if (!ValidParticle(id))
        return;

	float floatID = float(id);

	ParticleProperties[id].StartPosition = Random(
		StartPositionMin,
		StartPositionMax,
		floatID
	);

	ParticleProperties[id].StartColor = Random(
		StartColorMin,
		StartColorMax,
		floatID
	);

	ParticleProperties[id].StartRotation = Random(
		StartRotationMin,
		StartRotationMax,
		floatID
	);

	ParticleProperties[id].StartScale = Random(
		StartScaleMin,
		StartScaleMax,
		floatID
	);

	ParticleProperties[id].StartVelocity = Random(
		StartVelocityMin,
		StartVelocityMax,
		vec4(gl_GlobalInvocationID.x),
		vec4(gl_GlobalInvocationID.y),
		floatID
	);

	ParticleProperties[id].EndColor = Random(
		EndColorMin,
		EndColorMax,
		floatID
	);


	ParticleProperties[id].EndRotation = Random(
		EndRotationMin,
		EndRotationMax,
		floatID
	);

	ParticleProperties[id].EndScale = Random(
		EndScaleMin,
		EndScaleMax,
		floatID
	);


	ParticleProperties[id].EndVelocity = Random(
		EndVelocityMin,
		EndVelocityMax,
		floatID
	);

	ParticleProperties[id].LifeTime = Random(
		LifeTimeMin,
		LifeTimeMax,
		floatID
	);
}