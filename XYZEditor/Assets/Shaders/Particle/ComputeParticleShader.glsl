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

	float Alignment[1];
};

struct ParticleSpecification
{
	vec4  StartColor;		  
	vec2  StartSize;		  
	vec2  StartVelocity;
	vec2  StartPosition;

	float TimeAlive;
	int	  IsAlive;
};

layout(std430, binding = 1) buffer buffer_Data
{
	ParticleData Data[];
};

layout(std430, binding = 2) buffer buffer_Specification
{
	ParticleSpecification Specification[];
};

layout(std140, binding = 3) buffer buffer_DrawCommand
{
	DrawCommand Command;
};

layout(binding = 4, offset = 0) uniform atomic_uint counter_DeadParticles;


vec2 UpdatePosition(vec2 velocity, float speed, float ts)
{
	return vec2(velocity.x * speed * ts, velocity.y * speed * ts);
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
uniform int   u_ParticlesEmitted;

uniform float u_Time;
uniform float u_Gravity;
uniform float u_Speed;
uniform float u_LifeTime;

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
	specs.TimeAlive				+= u_Time;
	if (specs.TimeAlive > u_LifeTime)
	{
		specs.TimeAlive = 0.0;
		specs.IsAlive   = u_Repeat;
		data.Position   = specs.StartPosition;
		if (u_Repeat == 0)
			atomicCounterIncrement(counter_DeadParticles);
	}
	float ratio    = specs.TimeAlive / u_LifeTime;
	vec2  velocity = mix(specs.StartVelocity, specs.StartVelocity * u_VelocityRatio, ratio);
	data.Color	   = mix(specs.StartColor, specs.StartColor * u_ColorRatio, ratio);
	data.Size	   = mix(specs.StartSize, specs.StartSize * u_SizeRatio, ratio);
	data.Position  += UpdatePosition(velocity, u_Speed, u_Time);

	BuildDrawCommand(u_ParticlesEmitted);

	Data[id]		  = data;
	Specification[id] = specs;
}