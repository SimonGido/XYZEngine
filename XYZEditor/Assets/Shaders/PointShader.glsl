#type vertex
#version 430 core

layout(location = 0) in vec4 a_Color;
layout(location = 1) in vec3 a_Position;
layout(location = 2) in float a_Radius;


out vec4 v_Color;
out float v_Radius;

uniform mat4 u_ViewProjectionMatrix;

void main()
{
	v_Color = a_Color;
	v_Radius = a_Radius;

	gl_Position = u_ViewProjectionMatrix * vec4(a_Position, 1.0);
	gl_PointSize = a_Radius;
}

#type fragment
#version 430 core

layout(location = 0) out vec4 o_Color;

in vec4 v_Color;
in float v_Radius;

void main()
{
	vec2 circCoord = 2.0 * gl_PointCoord - 1.0;
	if (dot(circCoord, circCoord) > v_Radius) 
	    discard;

	o_Color = v_Color;
}
