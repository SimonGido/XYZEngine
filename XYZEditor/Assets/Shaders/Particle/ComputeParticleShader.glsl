//#type compute
#version 460

const int c_MaxParticles = 100;

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


layout(std140, binding = 0) uniform Camera
{
	mat4 u_ViewProjection;
};

layout(std430, binding = 1) buffer buffer_Data
{
	ParticleData Data[];
};

layout(std430, binding = 2) buffer buffer_Specification
{
	ParticleSpecification Specification[];
};

layout(std430, binding = 3) buffer buffer_BoxCollider
{
	vec4 BoxColliders[];
};

layout(std140, binding = 4) buffer buffer_DrawCommand
{
	DrawCommand Command;
};

layout(binding = 5, offset = 0) uniform atomic_uint counter_DeadParticles;


struct ColorOverLifeTime
{
	vec4 StartColor;
	vec4 EndColor;
};

void UpdateColorOverLifeTime(out vec4 result, vec4 color, ColorOverLifeTime colt, float ratio)
{
	result = color * mix(colt.StartColor, colt.EndColor, ratio);
}

struct SizeOverLifeTime
{
	vec2 StartSize;
	vec2 EndSize;
};

void UpdateSizeOverLifeTime(out vec2 result, vec2 size, SizeOverLifeTime solt, float ratio)
{
	result = size * mix(solt.StartSize, solt.EndSize, ratio);
}

struct TextureSheetAnimation
{
	int TilesX;
	int TilesY;
};

void UpdateTextureSheetOverLifeTime(out vec2 texCoord, float tilesX, float tilesY, float ratio)
{
	float stageCount = tilesX * tilesY;
	float stageProgress = ratio * stageCount;

	int index  = int(floor(stageProgress));
	int column = int(mod(index, tilesX));
	int row	   = int(index / tilesY);
	texCoord = vec2(float(column) / tilesX, float(row) / tilesY);
}

uniform mat4 u_Transform;

bool CollideBox(vec2 particlePos, vec2 particleSize, vec4 box)
{
	vec4 realPos = u_Transform * vec4(particlePos.x, particlePos.y, 0.0, 1.0);
	vec4 particleBox = vec4(realPos.x - particleSize.x / 2.0, realPos.y - particleSize.y / 2.0, 
							realPos.x + particleSize.x / 2.0, realPos.y + particleSize.y / 2.0);

	return (
		   particleBox.x <= box.z
		&& particleBox.z >= box.x
		&& particleBox.y <= box.w
		&& particleBox.w >= box.y
	);
}

struct Main
{
	int   Repeat;
	int   ParticlesEmitted;
	float Time;
	float Gravity;
	float Speed;
	float LifeTime;
};

void UpdatePosition(inout vec2 result, vec2 velocity, float speed, float ts)
{
	result += vec2(velocity.x * speed * ts, velocity.y * speed * ts);
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


uniform ColorOverLifeTime	  u_ColorModule;
uniform SizeOverLifeTime	  u_SizeModule;
uniform TextureSheetAnimation u_TextureModule;
uniform Main			      u_MainModule;
uniform vec2				  u_Force;
uniform int					  u_NumBoxColliders;

void KillParticle(inout float timeAlive, inout int isAlive, inout vec2 position, vec2 startPosition)
{	
	timeAlive = 0.0;
	isAlive = u_MainModule.Repeat;
	position = startPosition;
	if (u_MainModule.Repeat == 0)
		atomicCounterIncrement(counter_DeadParticles);
}


layout(local_size_x = 32, local_size_y = 32, local_size_z = 1) in;
void main(void)
{
	uint id = gl_GlobalInvocationID.x + gl_GlobalInvocationID.y * gl_NumWorkGroups.x * gl_WorkGroupSize.x;
	if (id > c_MaxParticles || Specification[id].IsAlive == 0)
		return;

	ParticleData data			= Data[id];
	ParticleSpecification specs = Specification[id];
	specs.TimeAlive				+= u_MainModule.Time;
	if (specs.TimeAlive > u_MainModule.LifeTime)
		KillParticle(specs.TimeAlive, specs.IsAlive, data.Position, specs.StartPosition);
	
	for (int i = 0; i < u_NumBoxColliders; ++i)
	{
		if (CollideBox(data.Position, data.Size, BoxColliders[i]))
		{
			KillParticle(specs.TimeAlive, specs.IsAlive, data.Position, specs.StartPosition);
			break;
		}
	}

	float ratio   = specs.TimeAlive / u_MainModule.LifeTime;
	vec2 velocity = specs.StartVelocity + (u_Force * ratio);
	UpdateColorOverLifeTime(data.Color, specs.StartColor, u_ColorModule, ratio);
	UpdateSizeOverLifeTime(data.Size, specs.StartSize, u_SizeModule, ratio);
	UpdatePosition(data.Position, velocity, u_MainModule.Speed, u_MainModule.Time);
	UpdateTextureSheetOverLifeTime(data.TexCoord, u_TextureModule.TilesX, u_TextureModule.TilesY, ratio);

	BuildDrawCommand(u_MainModule.ParticlesEmitted);
	
	Data[id]		  = data;
	Specification[id] = specs;
}