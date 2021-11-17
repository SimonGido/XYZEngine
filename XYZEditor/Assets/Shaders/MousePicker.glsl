#type vertex
#version 430 core

layout(location = 0) in vec3 a_Position;
layout(location = 1) in int a_ObjectID;

layout (location = 0) out flat int v_ObjectID;

layout(std140, binding = 0) uniform Camera
{
	mat4 u_ViewProjection;
	mat4 u_ViewMatrix;
	vec4 u_ViewPosition;
};

void main()
{
	v_ObjectID = a_ObjectID;
	gl_Position = u_ViewProjection * vec4(a_Position, 1.0);
}


#type fragment
#version 430

layout(location = 2) out int o_IDBuffer;

layout(location = 0) in flat int v_ObjectID;

void main()
{
	o_IDBuffer = v_ObjectID;
}