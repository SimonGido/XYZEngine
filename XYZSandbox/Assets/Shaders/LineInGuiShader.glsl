#type vertex
#version 430 core

layout(location = 0) in vec3 a_Position;
layout(location = 1) in vec4 a_Color;


out vec4 v_Color;


uniform vec2 u_ViewportSize;
uniform mat4 u_ViewProjection;

void main()
{
	v_Color = a_Color;
	gl_Position = u_ViewProjection * vec4(a_Position.xy / (u_ViewportSize / 2), 0.0, 1.0);
}

#type fragment
#version 430 core

layout(location = 0) out vec4 color;

in vec4 v_Color;

void main()
{
	color = v_Color;
}
