#type vertex
#version 430 core

layout(location = 0) in vec3 a_Position;
layout(location = 1) in vec4 a_Color;
layout(location = 2) in float a_ScissorIndex;


out vec4 v_Color;
out flat float v_ScissorIndex;

layout(std140, binding = 0) uniform Camera
{
	mat4 u_ViewProjection;
};

void main()
{
	v_Color = a_Color;
	v_ScissorIndex = a_ScissorIndex;
	gl_Position = u_ViewProjection * vec4(a_Position, 1.0);
}

#type fragment
#version 430 core

layout(location = 0) out vec4 color;

struct ScissorQuad
{
	float X;
	float Y;
	float Width;
	float Height;
};

layout(std430, binding = 0) buffer
buffer_ScissorQuads
{
	ScissorQuad Scissors[];
};


in vec4 v_Color;
in flat float v_ScissorIndex;

void main()
{
	if ((gl_FragCoord.x < Scissors[int(v_ScissorIndex)].X
		|| gl_FragCoord.x > Scissors[int(v_ScissorIndex)].X + Scissors[int(v_ScissorIndex)].Width)
		|| (gl_FragCoord.y < Scissors[int(v_ScissorIndex)].Y
		|| gl_FragCoord.y > Scissors[int(v_ScissorIndex)].Y + Scissors[int(v_ScissorIndex)].Height))
	{
		discard;
	}
	color = v_Color;
}
