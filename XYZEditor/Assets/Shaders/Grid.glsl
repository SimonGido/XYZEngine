#type vertex
#version 430

layout(location = 0) in vec3 a_Position;
layout(location = 1) in vec2 a_TexCoord;

layout(std140, binding = 0) uniform Camera
{
	mat4 u_ViewProjection;
};

uniform mat4 u_Transform;

out vec2 v_TexCoord;

void main()
{
	vec4 position = u_ViewProjection * u_Transform * vec4(a_Position.xy, 0.0, 1.0);
	gl_Position = position;

	v_TexCoord = a_TexCoord;
}

#type fragment
#version 430

layout(location = 0) out vec4 o_Color;

uniform vec2 u_Scale;
uniform float u_LineWidth;

in vec2 v_TexCoord;

float grid(vec2 st, float lineWidth)
{
	vec2 grid = fract(st);
	return step(lineWidth, grid.x) * step(lineWidth, grid.y);
}

void main()
{
	float x = grid(v_TexCoord * u_Scale, u_LineWidth);
	o_Color = vec4(vec3(0.2), 1.0) * (1.0 - x);
}
