#type vertex
#version 430

layout(location = 0) in vec3 a_Position;
layout(location = 1) in vec2 a_TexCoord;

layout(std140, binding = 0) uniform Camera
{
	mat4 u_ViewProjection;
	mat4 u_Projection;
	mat4 u_View;
};

layout (push_constant) uniform Transform
{
	mat4 Transform;
} u_Renderer;


layout (location = 0) out vec2 v_TexCoord;

void main()
{
	vec4 position = u_ViewProjection * u_Renderer.Transform * vec4(a_Position.xy, 0.0, 1.0);
	gl_Position = position;

	v_TexCoord = a_TexCoord;
}

#type fragment
#version 430

layout(location = 0) out vec4 o_Color;
layout(location = 1) out vec4 o_Position;

layout (push_constant) uniform Settings
{
	layout (offset = 64) float Scale;
	float Size;
} u_Settings;

layout (location = 0) in vec2 v_TexCoord;

float grid(vec2 st, float lineWidth)
{
	vec2 grid = fract(st);
	return step(lineWidth, grid.x) * step(lineWidth, grid.y);
}

void main()
{
	float x = grid(v_TexCoord * u_Settings.Scale, u_Settings.Size);
	o_Color = vec4(vec3(0.2), 1.0) * (1.0 - x);
	o_Position = vec4(0.0, 0.0, 0.0, 0.0);

	if (o_Color.a == 0.0)
		discard;
}
