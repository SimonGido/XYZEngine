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

struct BoxCollider
{
	vec2 Min;
	vec2 Max;
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

layout(std140, binding = 4) buffer buffer_BoxCollider
{
	BoxCollider BoxColliders[];
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


bool CollideBox(vec2 particlePos, vec2 particleSize,  BoxCollider box)
{
	BoxCollider particleBox;
	particleBox.Min = vec2(particlePos.x - particleSize.x / 2.0, particlePos.y - particleSize.y / 2.0);
	particleBox.Max = vec2(particlePos.x + particleSize.x / 2.0, particlePos.y + particleSize.y / 2.0);
	
	return !(
		   particleBox.Min.x < box.Max.x
		&& particleBox.Max.x > box.Min.x
		&& particleBox.Min.y < box.Max.y
		&& particleBox.Max.y > box.Min.y
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



uniform ColorOverLifeTime	  u_Color;
uniform SizeOverLifeTime	  u_Size;
uniform TextureSheetAnimation u_TextureAnimation;
uniform Main			      u_Main;
uniform int					  u_NumBoxColliders;

layout(local_size_x = 32, local_size_y = 32, local_size_z = 1) in;
void main(void)
{
	uint id = gl_GlobalInvocationID.x + gl_GlobalInvocationID.y * gl_NumWorkGroups.x * gl_WorkGroupSize.x;
	if (id > c_MaxParticles || Specification[id].IsAlive == 0)
		return;

	ParticleData data			= Data[id];
	ParticleSpecification specs = Specification[id];
	specs.TimeAlive				+= u_Main.Time;
	if (specs.TimeAlive > u_Main.LifeTime)
	{
		specs.TimeAlive = 0.0;
		specs.IsAlive   = u_Main.Repeat;
		data.Position   = specs.StartPosition;
		if (u_Main.Repeat == 0)
			atomicCounterIncrement(counter_DeadParticles);
	}

	float ratio    = specs.TimeAlive / u_Main.LifeTime;
	UpdateColorOverLifeTime(data.Color, specs.StartColor, u_Color, ratio);
	UpdateSizeOverLifeTime(data.Size, specs.StartSize, u_Size, ratio);
	UpdatePosition(data.Position, specs.StartVelocity, u_Main.Speed, u_Main.Time);
	UpdateTextureSheetOverLifeTime(data.TexCoord, u_TextureAnimation.TilesX, u_TextureAnimation.TilesY, ratio);

	BuildDrawCommand(u_Main.ParticlesEmitted);
	
	Data[id]		  = data;
	Specification[id] = specs;
}