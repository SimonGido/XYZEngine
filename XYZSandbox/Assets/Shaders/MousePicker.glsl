#type vertex
#version 430 core

layout(location = 0) in vec3 a_Position;
layout(location = 1) in int a_ObjectID;

out flat int v_ObjectID;

uniform mat4 u_ViewProjectionMatrix;

void main()
{
	v_ObjectID = a_ObjectID;
	gl_Position = u_ViewProjectionMatrix * vec4(a_Position, 1.0);
}


#type fragment
#version 430

layout(location = 2) out int o_IDBuffer;

in flat int a_ObjectID;

void main()
{
	o_IDBuffer = a_ObjectID;
}