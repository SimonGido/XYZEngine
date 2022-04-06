#type vertex
#version 450 core

layout(location = 0) in vec3  a_Position;
layout(location = 1) in vec2  a_TexCoord;


layout (std140, binding = 0) uniform Camera
{
	mat4 u_ViewProjection;
	mat4 u_InverseView;
};

layout(push_constant) uniform Transform
{
	mat4 Transform;
} u_Renderer;


struct VertexOutput
{
	vec3 fEye;
	vec3 fPos;
};

layout (location = 0) out VertexOutput v_Output;

void main()
{
    gl_Position = u_Renderer.Transform * vec4(a_Position, 1.0);
	v_Output.fPos = a_Position;
	v_Output.fEye = a_Position - (u_InverseView  * vec4(vec3(0), 1)).xyz; 
}


#type fragment
#version 450

layout(location = 0) out vec4 o_Color;


const float voxelSize = 0.25;
const float voxelGridSize = 32;


layout(std430, binding = 3) buffer buffer_Voxels
{
	int Voxels[32][32][32];
};

float maxcomp(in vec3 p ) { return max(p.x,max(p.y,p.z));}
float maxcomp(in vec2 p ) { return max(p.x,p.y);}

float mincomp(in vec3 p ) { return min(p.x,min(p.y,p.z));}

vec4 march(vec3 startPoint, vec3 dir) 
{
	float t = 0;

	float endT = mincomp(max((vec3(voxelGridSize * voxelSize) - startPoint) / dir, -startPoint / dir));

	while (t <= endT) 
	{
		vec3 p = startPoint + dir * t;
		vec3 index = p / (voxelSize * voxelGridSize);
		int solid = Voxels[int(index.x)][int(index.y)][int(index.z)];

		if (solid == 1)
		{
			return vec4(1,0,1,1);
		}
		t += voxelSize - mincomp(abs(mod(p, voxelSize)));
	}

	return vec4(0);
}

struct VertexOutput
{
	vec3 fEye;
	vec3 fPos;
};

layout (location = 0) in VertexOutput v_Input;

void main() 
{
	vec3 eyeNormal = normalize(v_Input.fEye);

	vec4 color = march(vec3(0,0,0), eyeNormal);
	if (color.a == 0) 
	{
		discard;
	}

	o_Color = color;
}