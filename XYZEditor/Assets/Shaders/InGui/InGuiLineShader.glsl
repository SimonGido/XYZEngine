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
	vec2 Min;
	vec2 Max;
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
	ScissorQuad rect = Scissors[int(v_ScissorIndex)];
	if ((gl_FragCoord.x < rect.Min.x 
	 ||  gl_FragCoord.x > rect.Max.x)
	 || (gl_FragCoord.y < rect.Min.y 
	 ||  gl_FragCoord.y > rect.Max.y))
	{
		discard;
	}
	color = v_Color;
}
