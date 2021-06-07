//#type compute
#version 460

const int c_MaxParticles = 100000;

struct DrawCommand 
{
	uint Count;         
	uint InstanceCount; 
	uint FirstIndex;    
	uint BaseVertex;    
	uint BaseInstance;  
};


struct ParticleData
{
	vec4  Color;
	vec2  Position;
	vec2  TexCoord;
	vec2  Size;
	float Rotation;

	float Alignment[3];
};

struct ParticleSpecification
{
	vec4  StartColor;		  
	vec2  StartSize;		  
	vec2  StartVelocity;
	vec2  StartPosition;

	float LifeTime;
	float TimeAlive;
	int	  IsAlive;
	
	float Allignment[3];
};

layout(std430, binding = 0) buffer buffer_Data
{
	ParticleData Data[];
};

layout(std430, binding = 1) buffer buffer_Specification
{
	ParticleSpecification Specification[];
};

layout(std140, binding = 2) buffer buffer_DrawCommand
{
	DrawCommand Command;
};

layout(binding = 3, offset = 0) uniform atomic_uint counter_DeadParticles;


vec2 UpdatePosition(vec2 velocity, float speed, float ts)
{
	return vec2(velocity.x * speed * ts, velocity.y * speed * ts);
}

bool RestartParticle(int repeat, out float timeAlive, out int isAlive)
{
	if (repeat != 0)
	{
		timeAlive = 0.0f;
	}
	else
	{
		atomicCounterIncrement(counter_DeadParticles);
	}
	isAlive = repeat;
	return isAlive != 0;
}

void BuildDrawCommand(int particlesInExistence)
{
	uint deadCount = atomicCounter(counter_DeadParticles);
	Command.Count = 6;
	Command.InstanceCount = particlesInExistence - deadCount;
	Command.FirstIndex = 0;
	Command.BaseVertex = 0;
	Command.BaseInstance = deadCount;
}


uniform int   u_Repeat;
uniform int   u_ParticlesInExistence;

uniform float u_Time;
uniform float u_Gravity;
uniform float u_Speed;

uniform vec4 u_ColorRatio;
uniform vec2 u_SizeRatio;
uniform vec2 u_VelocityRatio;

layout(local_size_x = 32, local_size_y = 32, local_size_z = 1) in;
void main(void)
{
	uint id = gl_GlobalInvocationID.x + gl_GlobalInvocationID.y * gl_NumWorkGroups.x * gl_WorkGroupSize.x;
	if (id > c_MaxParticles || Specification[id].IsAlive == 0)
		return;

	ParticleData data			= Data[id];
	ParticleSpecification specs = Specification[id];

	if (specs.TimeAlive > specs.LifeTime)
	{
		if (RestartParticle(u_Repeat, specs.TimeAlive, specs.LifeTime))
			data.Position = specs.StartPosition;
	}
	float ratio = specs.TimeAlive / specs.LifeTime;
	vec2  velocity = mix(specs.StartVelocity, specs.StartVelocity * u_VelocityRatio, ratio);
	data.Color	   = mix(specs.StartColor, specs.StartColor * u_ColorRatio, ratio);
	data.Size	   = mix(specs.StartSize, specs.StartSize * u_SizeRatio, ratio);
	data.Position  = UpdatePosition(velocity, u_Speed, u_Time);

	BuildDrawCommand(u_ParticlesInExistence);

	Data[id]		  = data;
	Specification[id] = specs;
}