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
	
	float Alignment[2];
};

struct ParticleSpecification
{
	vec4  StartColor;
	vec4  EndColor;
		  
	vec2  StartSize;
	vec2  EndSize;
		  
	vec2  StartVelocity;
	vec2  EndVelocity;

	float LifeTime;
	float TimeAlive;
	int	  IsAlive;
	
	float Allignment[1];
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



vec4 ChangeColorOverLife(vec4 startColor, vec4 endColor, float timeAlive, float lifeTime)
{
	return mix(startColor, endColor, timeAlive / lifeTime);
}

vec2 ChangeSizeOverLife(vec2 startSize, vec2 endSize, float timeAlive, float lifeTime)
{
	return mix(startSize, endSize, timeAlive / lifeTime);
}

vec2 ChangeVelocityOverLife(vec2 startvelocity, vec2 endVelocity, float timeAlive, float lifeTime)
{
	return mix(startvelocity, endVelocity, timeAlive / lifeTime);
}

vec2 UpdatePosition(vec2 velocity, float speed, float ts)
{
	return vec2(velocity.x * speed * ts, velocity.y * speed * ts);
}

void RestartParticle(int loop, out float timeAlive, out int isAlive)
{
	if (loop != 0)
	{
		timeAlive = 0.0f;
	}
	else
	{
		atomicCounterIncrement(counter_DeadParticles);
	}
	isAlive = loop;
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


uniform int   u_Loop;
uniform int   u_ParticlesInExistence;
uniform float u_Time;
uniform float u_Gravity;
uniform float u_Speed;

layout(local_size_x = 32, local_size_y = 32, local_size_z = 1) in;
void main(void)
{
	uint id = gl_GlobalInvocationID.x + gl_GlobalInvocationID.y * gl_NumWorkGroups.x * gl_WorkGroupSize.x;
	if (id > c_MaxParticles || Specification[id].IsAlive == 0)
		return;

	ParticleData data			= Data[id];
	ParticleSpecification specs = Specification[id];

	if (specs.TimeAlive > specs.LifeTime)
		RestartParticle(u_Loop, specs.TimeAlive, specs.LifeTime);
	data.Position = UpdatePosition(ChangeVelocityOverLife(specs.StartVelocity, specs.EndVelocity, specs.TimeAlive, specs.LifeTime), u_Speed, u_Time);
	data.Color	  = ChangeColorOverLife(specs.StartColor, specs.EndColor, specs.TimeAlive, specs.LifeTime);
	data.Size	  = ChangeSizeOverLife(specs.StartSize, specs.EndSize, specs.TimeAlive, specs.LifeTime);
	BuildDrawCommand(u_ParticlesInExistence);

	Data[id]		  = data;
	Specification[id] = specs;
}